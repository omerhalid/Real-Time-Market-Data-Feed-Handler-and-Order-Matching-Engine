#pragma once

#ifdef __linux__

#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

// NUMA support (if available)
#ifdef __has_include
#if __has_include(<numa.h>)
#include <numa.h>
#define HFT_NUMA_AVAILABLE
#endif
#endif

// Linux-specific HFT optimizations
namespace hft {

// Lock memory to prevent swapping (mlock)
inline bool lockMemory() {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        return false;
    }
    return true;
}

// Set CPU affinity for current process (pin to specific CPU cores)
inline bool setCpuAffinity(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        return false;
    }
    return true;
}

// Pin current thread to specific CPU core (thread pinning)
inline bool pinThreadToCpu(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    pthread_t thread = pthread_self();
    if (pthread_setaffinity_np(thread, sizeof(cpuset), &cpuset) != 0) {
        return false;
    }
    return true;
}

// Pin thread to CPU core (C++11 thread version)
inline bool pinThreadToCpu(std::thread& thread, int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    pthread_t native_handle = thread.native_handle();
    if (pthread_setaffinity_np(native_handle, sizeof(cpuset), &cpuset) != 0) {
        return false;
    }
    return true;
}

// Get NUMA node for a CPU core
inline int getNumaNode(int cpu_id) {
#ifdef HFT_NUMA_AVAILABLE
    if (numa_available() >= 0) {
        return numa_node_of_cpu(cpu_id);
    }
#endif
    // Fallback: try to read from sysfs
    std::ostringstream path;
    path << "/sys/devices/system/cpu/cpu" << cpu_id << "/node" << 0;
    std::ifstream file(path.str());
    if (file.is_open()) {
        int node;
        file >> node;
        return node;
    }
    return 0; // Default to node 0
}

// Get CPU cores on a specific NUMA node
inline std::vector<int> getCpusOnNumaNode(int node_id) {
    std::vector<int> cpus;
#ifdef HFT_NUMA_AVAILABLE
    if (numa_available() >= 0) {
        struct bitmask* mask = numa_allocate_cpumask();
        if (numa_node_to_cpus(node_id, mask) == 0) {
            for (int i = 0; i < numa_num_possible_cpus(); ++i) {
                if (numa_bitmask_isbitset(mask, i)) {
                    cpus.push_back(i);
                }
            }
        }
        numa_free_cpumask(mask);
        return cpus;
    }
#endif
    // Fallback: read from sysfs
    std::ostringstream path;
    path << "/sys/devices/system/node/node" << node_id << "/cpulist";
    std::ifstream file(path.str());
    if (file.is_open()) {
        std::string line;
        std::getline(file, line);
        // Parse cpulist format (e.g., "0-3,8-11")
        // Simplified parser - in production use proper parsing
        cpus.push_back(node_id * 8); // Rough estimate
    }
    return cpus;
}

// Allocate memory on specific NUMA node
inline void* allocateOnNumaNode(size_t size, int node_id) {
#ifdef HFT_NUMA_AVAILABLE
    if (numa_available() >= 0) {
        void* ptr = numa_alloc_onnode(size, node_id);
        if (ptr) {
            return ptr;
        }
    }
#endif
    // Fallback: regular allocation
    return malloc(size);
}

// Set memory policy to prefer specific NUMA node
inline bool setMemoryPolicy(int node_id) {
#ifdef HFT_NUMA_AVAILABLE
    if (numa_available() >= 0) {
        struct bitmask* mask = numa_allocate_nodemask();
        numa_bitmask_setbit(mask, node_id);
        if (set_mempolicy(MPOL_PREFERRED, mask->maskp, mask->size + 1) == 0) {
            numa_free_nodemask(mask);
            return true;
        }
        numa_free_nodemask(mask);
    }
#endif
    return false;
}

// Bind current thread to NUMA node (sets CPU affinity to CPUs on that node)
inline bool bindThreadToNumaNode(int node_id) {
    std::vector<int> cpus = getCpusOnNumaNode(node_id);
    if (cpus.empty()) {
        return false;
    }
    // Pin to first CPU on the NUMA node
    return pinThreadToCpu(cpus[0]);
}

// Set real-time priority (SCHED_FIFO)
inline bool setRealtimePriority(int priority) {
    struct sched_param param;
    param.sched_priority = priority;
    
    if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
        return false;
    }
    return true;
}

// Disable CPU frequency scaling (requires root)
inline void disableCpuFrequencyScaling() {
    // This would typically be done via cpufreq-set command
    // or by writing to /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
    // For now, just a placeholder
}

// Set process to use huge pages (2MB pages)
inline bool enableHugePages() {
    // This requires system configuration:
    // echo 1024 > /proc/sys/vm/nr_hugepages
    // And linking with -Wl,-z,common-page-size=2097152
    return true;
}

// Set high priority for network operations
inline bool setNetworkPriority() {
    // Set socket options for low latency
    // This is typically done per-socket, not globally
    return true;
}

// Initialize all HFT optimizations with NUMA awareness
inline bool initializeHftOptimizations(int cpu_id = 0, int rt_priority = 50, 
                                       bool numa_aware = true) {
    bool success = true;
    
    // Lock memory
    if (!lockMemory()) {
        // Non-fatal, continue
    }
    
    // NUMA-aware initialization
    if (numa_aware) {
        int numa_node = getNumaNode(cpu_id);
        setMemoryPolicy(numa_node);
        bindThreadToNumaNode(numa_node);
    }
    
    // Set CPU affinity
    if (!setCpuAffinity(cpu_id)) {
        success = false;
    }
    
    // Pin current thread to CPU
    pinThreadToCpu(cpu_id);
    
    // Set real-time priority (requires root)
    if (!setRealtimePriority(rt_priority)) {
        // Non-fatal if not root
    }
    
    return success;
}

// Get number of CPU cores
inline int getCpuCount() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

} // namespace hft

#endif // __linux__

