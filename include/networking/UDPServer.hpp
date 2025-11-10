#pragma once

#ifdef __linux__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <cstring>
#include <functional>
#include <string>
#include "../utils/Timestamp.hpp"

// High-performance UDP server with busy polling for ultra-low latency
// Designed for market data feeds in HFT systems
class UDPServer {
public:
    using MessageHandler = std::function<void(const char* data, size_t len, 
                                               const struct sockaddr_in& from)>;
    
    UDPServer(uint16_t port, MessageHandler handler, bool busy_poll = true)
        : port_(port), handler_(handler), socket_fd_(-1), busy_poll_(busy_poll) {}
    
    ~UDPServer() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }
    
    // Initialize and bind socket
    bool start() {
        // Create UDP socket
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0) {
            return false;
        }
        
        // Set socket to non-blocking
        int flags = fcntl(socket_fd_, F_GETFL, 0);
        fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK);
        
        // Set socket options for low latency
        int opt = 1;
        setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        
        // Enable busy polling (SO_BUSY_POLL) - reduces latency by polling in kernel
        // This allows the kernel to poll the socket queue without going to sleep
        if (busy_poll_) {
            // SO_BUSY_POLL: microseconds to busy poll before blocking
            // Typical values: 50-200 microseconds
            int busy_poll_us = 50;
            setsockopt(socket_fd_, SOL_SOCKET, SO_BUSY_POLL, &busy_poll_us, sizeof(busy_poll_us));
            
            // SO_INCOMING_CPU: bind socket to specific CPU for better cache locality
            // This helps when using SO_REUSEPORT with multiple threads
            // int cpu_id = sched_getcpu();
            // setsockopt(socket_fd_, SOL_SOCKET, SO_INCOMING_CPU, &cpu_id, sizeof(cpu_id));
        }
        
        // Increase receive buffer size for high-throughput scenarios
        int rcvbuf_size = 1024 * 1024; // 1MB
        setsockopt(socket_fd_, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size));
        
        // Bind to address
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            return false;
        }
        
        return true;
    }
    
    // Receive messages with busy polling (non-blocking)
    // Returns number of messages received
    int receiveMessages(int max_messages = 64) {
        char buffer[65536]; // Max UDP packet size
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        int received = 0;
        
        for (int i = 0; i < max_messages; ++i) {
            ssize_t n = recvfrom(socket_fd_, buffer, sizeof(buffer), MSG_DONTWAIT,
                                 (struct sockaddr*)&from_addr, &from_len);
            
            if (n < 0) [[unlikely]] {
                if (errno == EAGAIN || errno == EWOULDBLOCK) [[likely]] {
                    break; // No more data
                }
                // Error - continue to next message
                continue;
            }
            
            // Process message
            handler_(buffer, n, from_addr);
            received++;
        }
        
        return received;
    }
    
    // Run receive loop with busy polling
    void run() {
        while (true) {
            int received = receiveMessages();
            
            if (busy_poll_ && received == 0) [[unlikely]] {
                // Busy poll: use poll() with 0 timeout for immediate return
                struct pollfd pfd;
                pfd.fd = socket_fd_;
                pfd.events = POLLIN;
                
                // Poll with 0 timeout (non-blocking) for busy polling
                poll(&pfd, 1, 0);
                
                // Small CPU pause to avoid 100% CPU usage
                // In production, might want to use _mm_pause() or similar
                __asm__ __volatile__("pause");
            } else if (received == 0) [[unlikely]] {
                // No busy polling: sleep briefly to avoid 100% CPU
                usleep(1); // 1 microsecond
            }
        }
    }
    
    // Send UDP packet
    bool sendTo(const char* data, size_t len, const struct sockaddr_in& to) {
        ssize_t sent = sendto(socket_fd_, data, len, MSG_DONTWAIT | MSG_NOSIGNAL,
                             (const struct sockaddr*)&to, sizeof(to));
        return sent == static_cast<ssize_t>(len);
    }
    
    // Get socket file descriptor (for integration with epoll/select)
    int getFd() const { return socket_fd_; }

private:
    uint16_t port_;
    MessageHandler handler_;
    int socket_fd_;
    bool busy_poll_;
};

// UDP Multicast receiver for market data feeds
class UDPMulticastReceiver {
public:
    using MessageHandler = std::function<void(const char* data, size_t len)>;
    
    UDPMulticastReceiver(const std::string& multicast_group, uint16_t port,
                        const std::string& interface_ip, MessageHandler handler,
                        bool busy_poll = true)
        : multicast_group_(multicast_group), port_(port), interface_ip_(interface_ip),
          handler_(handler), socket_fd_(-1), busy_poll_(busy_poll) {}
    
    ~UDPMulticastReceiver() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }
    
    bool start() {
        // Create UDP socket
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0) {
            return false;
        }
        
        // Set to non-blocking
        int flags = fcntl(socket_fd_, F_GETFL, 0);
        fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK);
        
        // Enable SO_REUSEADDR and SO_REUSEPORT for multiple receivers
        int opt = 1;
        setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        
        // Enable busy polling
        if (busy_poll_) {
            int busy_poll_us = 50;
            setsockopt(socket_fd_, SOL_SOCKET, SO_BUSY_POLL, &busy_poll_us, sizeof(busy_poll_us));
        }
        
        // Bind to multicast port
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            return false;
        }
        
        // Join multicast group
        struct ip_mreq mreq;
        inet_aton(multicast_group_.c_str(), &mreq.imr_multiaddr);
        inet_aton(interface_ip_.c_str(), &mreq.imr_interface);
        
        if (setsockopt(socket_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
            return false;
        }
        
        return true;
    }
    
    int receiveMessages(int max_messages = 64) {
        char buffer[65536];
        int received = 0;
        
        for (int i = 0; i < max_messages; ++i) {
            ssize_t n = recv(socket_fd_, buffer, sizeof(buffer), MSG_DONTWAIT);
            
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                continue;
            }
            
            handler_(buffer, n);
            received++;
        }
        
        return received;
    }
    
    void run() {
        while (true) {
            int received = receiveMessages();
            
            if (busy_poll_ && received == 0) {
                struct pollfd pfd;
                pfd.fd = socket_fd_;
                pfd.events = POLLIN;
                poll(&pfd, 1, 0);
                __asm__ __volatile__("pause");
            } else if (received == 0) {
                usleep(1);
            }
        }
    }
    
    int getFd() const { return socket_fd_; }

private:
    std::string multicast_group_;
    uint16_t port_;
    std::string interface_ip_;
    MessageHandler handler_;
    int socket_fd_;
    bool busy_poll_;
};

#endif // __linux__

