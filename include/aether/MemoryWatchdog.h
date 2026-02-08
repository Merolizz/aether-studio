#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

namespace aether {

class MemoryWatchdog {
public:
    using WarningCallback = std::function<void(uint64_t usedMB, uint64_t totalMB, float percentage)>;
    using CleanupCallback = std::function<void()>;
    
    // Singleton access
    static MemoryWatchdog& getInstance();
    
    // Delete copy constructor and assignment operator
    MemoryWatchdog(const MemoryWatchdog&) = delete;
    MemoryWatchdog& operator=(const MemoryWatchdog&) = delete;
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Monitoring
    void update();
    void startMonitoring();
    void stopMonitoring();
    
    // Threshold management
    void setWarningThreshold(float percentage); // Default 0.75 (75%)
    float getWarningThreshold() const { return m_warningThreshold; }
    
    // Status
    bool isOverThreshold() const;
    uint64_t getTotalRAM() const;
    uint64_t getUsedRAM() const;
    float getRAMUsagePercentage() const;
    
    // Callbacks
    void setWarningCallback(WarningCallback callback) { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_warningCallback = callback; 
    }
    void setCleanupCallback(CleanupCallback callback) { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cleanupCallback = callback; 
    }
    
    // Cleanup operations
    void performCleanup();
    void requestCleanup();

private:
    MemoryWatchdog() = default;
    ~MemoryWatchdog() = default;
    
    // Monitoring thread
    void monitoringThread();
    void triggerWarning();
    
    // Cleanup operations
    void cleanupUnusedWorkspaces();
    void cleanupInactiveResources();
    
    mutable std::mutex m_mutex;
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_monitoring{false};
    std::atomic<bool> m_shouldStop{false};
    std::atomic<bool> m_cleanupRequested{false};
    
    float m_warningThreshold = 0.75f; // 75% default
    
    WarningCallback m_warningCallback;
    CleanupCallback m_cleanupCallback;
    
    std::thread m_monitoringThread;
    std::chrono::steady_clock::time_point m_lastWarningTime;
    bool m_warningTriggered = false;
    
    // Monitoring interval (milliseconds)
    static constexpr int MONITORING_INTERVAL_MS = 1000; // Check every second
};

} // namespace aether
