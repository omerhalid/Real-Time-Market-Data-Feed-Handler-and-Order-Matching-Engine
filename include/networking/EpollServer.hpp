#pragma once

#ifdef __linux__

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <vector>
#include <memory>
#include <functional>
#include "../order_matching/OrderBook.hpp"
#include "../order_matching/Matcher.hpp"
#include "../utils/Timestamp.hpp"

// High-performance epoll-based server for Linux
// Designed for low-latency order processing
class EpollServer {
public:
    using MessageHandler = std::function<void(int fd, const char* data, size_t len)>;
    
    EpollServer(uint16_t port, MessageHandler handler) 
        : port_(port), handler_(handler), epoll_fd_(-1), listen_fd_(-1) {}
    
    ~EpollServer() {
        if (epoll_fd_ >= 0) close(epoll_fd_);
        if (listen_fd_ >= 0) close(listen_fd_);
    }
    
    // Initialize and start listening
    bool start() {
        // Create epoll instance
        epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd_ < 0) {
            return false;
        }
        
        // Create listening socket
        listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (listen_fd_ < 0) {
            return false;
        }
        
        // Set socket options for low latency
        int opt = 1;
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
        
        // Disable Nagle's algorithm for low latency
        opt = 1;
        setsockopt(listen_fd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
        
        // Bind and listen
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port_);
        
        if (bind(listen_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            return false;
        }
        
        if (listen(listen_fd_, SOMAXCONN) < 0) {
            return false;
        }
        
        // Add listening socket to epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET; // Edge-triggered
        ev.data.fd = listen_fd_;
        epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
        
        return true;
    }
    
    // Run event loop (blocking)
    void run() {
        constexpr int MAX_EVENTS = 64;
        struct epoll_event events[MAX_EVENTS];
        
        while (true) {
            int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }
            
            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == listen_fd_) {
                    handleNewConnection();
                } else {
                    handleClientData(events[i].data.fd);
                }
            }
        }
    }
    
    // Process events (non-blocking, for integration with other loops)
    void processEvents(int timeout_ms = 0) {
        constexpr int MAX_EVENTS = 64;
        struct epoll_event events[MAX_EVENTS];
        
        int nfds = epoll_wait(epoll_fd_, events, MAX_EVENTS, timeout_ms);
        if (nfds < 0) {
            if (errno == EINTR) return;
            return;
        }
        
        for (int i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listen_fd_) {
                handleNewConnection();
            } else {
                handleClientData(events[i].data.fd);
            }
        }
    }

private:
    uint16_t port_;
    MessageHandler handler_;
    int epoll_fd_;
    int listen_fd_;
    
    void handleNewConnection() {
        while (true) {
            struct sockaddr_in addr;
            socklen_t addr_len = sizeof(addr);
            int client_fd = accept4(listen_fd_, (struct sockaddr*)&addr, &addr_len, 
                                    SOCK_NONBLOCK | SOCK_CLOEXEC);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break; // No more connections
                }
                continue;
            }
            
            // Set TCP_NODELAY for low latency
            int opt = 1;
            setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
            
            // Add to epoll
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
            ev.data.fd = client_fd;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev);
        }
    }
    
    void handleClientData(int fd) {
        // Read data (simplified - in production, use proper buffering)
        char buffer[4096];
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        
        if (n <= 0) {
            if (n == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)) {
                // Connection closed or error
                epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
            }
            return;
        }
        
        buffer[n] = '\0';
        handler_(fd, buffer, n);
    }
};

#endif // __linux__

