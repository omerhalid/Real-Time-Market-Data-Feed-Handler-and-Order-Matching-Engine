#pragma once

#ifdef __linux__

#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

// Linux-specific HFT optimizations
namespace hft {

// Lock memory to prevent swapping (mlock)
inline bool lockMemory() {
    if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
        return false;
    }
    return true;
}

// Set CPU affinity (pin to specific CPU cores)
inline bool setCpuAffinity(int cpu_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        return false;
    }
    return true;
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

// Initialize all HFT optimizations
inline bool initializeHftOptimizations(int cpu_id = 0, int rt_priority = 50) {
    bool success = true;
    
    // Lock memory
    if (!lockMemory()) {
        // Non-fatal, continue
    }
    
    // Set CPU affinity
    if (!setCpuAffinity(cpu_id)) {
        success = false;
    }
    
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

