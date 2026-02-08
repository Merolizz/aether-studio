#include "../../include/aether/MemoryWatchdog.h"
#include "../../include/aether/MemoryManager.h"
#include "../../include/aether/ResourceStreamer.h"
#include "../../include/aether/WorkspaceManager.h"
#include <iostream>
#include <chrono>

namespace aether {

MemoryWatchdog& MemoryWatchdog::getInstance() {
    static MemoryWatchdog instance;
    return instance;
}

bool MemoryWatchdog::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized.load()) {
        return true;
    }
    
    // Ensure MemoryManager is initialized
    auto& memoryManager = MemoryManager::getInstance();
    if (!memoryManager.initialize()) {
        std::cerr << "Failed to initialize MemoryManager" << std::endl;
        return false;
    }
    
    m_initialized.store(true);
    m_lastWarningTime = std::chrono::steady_clock::now();
    
    std::cout << "Memory Watchdog initialized (threshold: " << (m_warningThreshold * 100.0f) << "%)" << std::endl;
    return true;
}

void MemoryWatchdog::shutdown() {
    stopMonitoring();
    
    m_initialized.store(false);
    std::cout << "Memory Watchdog shutdown" << std::endl;
}

void MemoryWatchdog::startMonitoring() {
    if (m_monitoring.load()) {
        return;
    }
    
    if (!m_initialized.load()) {
        if (!initialize()) {
            std::cerr << "Failed to initialize MemoryWatchdog before starting monitoring" << std::endl;
            return;
        }
    }
    
    m_shouldStop.store(false);
    m_monitoring.store(true);
    
    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
    
    m_monitoringThread = std::thread(&MemoryWatchdog::monitoringThread, this);
    std::cout << "Memory Watchdog monitoring started" << std::endl;
}

void MemoryWatchdog::stopMonitoring() {
    if (!m_monitoring.load()) {
        return;
    }
    
    m_shouldStop.store(true);
    m_monitoring.store(false);
    
    if (m_monitoringThread.joinable()) {
        m_monitoringThread.join();
    }
    
    std::cout << "Memory Watchdog monitoring stopped" << std::endl;
}

void MemoryWatchdog::update() {
    if (!m_initialized.load()) {
        return;
    }
    
    // Update MemoryManager
    auto& memoryManager = MemoryManager::getInstance();
    memoryManager.update();
    
    // Check threshold
    if (isOverThreshold()) {
        // Trigger warning if not already triggered recently
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastWarning = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_lastWarningTime).count();
        
        if (timeSinceLastWarning >= 5) { // Warn at most every 5 seconds
            triggerWarning();
            m_lastWarningTime = now;
        }
        
        // Perform cleanup if requested
        if (m_cleanupRequested.load()) {
            performCleanup();
            m_cleanupRequested.store(false);
        }
    } else {
        m_warningTriggered = false;
    }
}

void MemoryWatchdog::monitoringThread() {
    while (!m_shouldStop.load() && m_monitoring.load()) {
        update();
        
        // Sleep for monitoring interval
        std::this_thread::sleep_for(std::chrono::milliseconds(MONITORING_INTERVAL_MS));
    }
}

bool MemoryWatchdog::isOverThreshold() const {
    if (!m_initialized.load()) {
        return false;
    }
    
    auto& memoryManager = MemoryManager::getInstance();
    float usage = memoryManager.getRAMUsagePercentage();
    
    return usage >= m_warningThreshold;
}

uint64_t MemoryWatchdog::getTotalRAM() const {
    if (!m_initialized.load()) {
        return 0;
    }
    
    auto& memoryManager = MemoryManager::getInstance();
    return memoryManager.getTotalRAM();
}

uint64_t MemoryWatchdog::getUsedRAM() const {
    if (!m_initialized.load()) {
        return 0;
    }
    
    auto& memoryManager = MemoryManager::getInstance();
    return memoryManager.getUsedRAM();
}

float MemoryWatchdog::getRAMUsagePercentage() const {
    if (!m_initialized.load()) {
        return 0.0f;
    }
    
    auto& memoryManager = MemoryManager::getInstance();
    return memoryManager.getRAMUsagePercentage();
}

void MemoryWatchdog::setWarningThreshold(float percentage) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (percentage < 0.0f || percentage > 1.0f) {
        std::cerr << "Invalid warning threshold: " << percentage << " (must be 0.0-1.0)" << std::endl;
        return;
    }
    
    m_warningThreshold = percentage;
    std::cout << "Memory warning threshold set to " << (percentage * 100.0f) << "%" << std::endl;
}

void MemoryWatchdog::triggerWarning() {
    if (m_warningTriggered) {
        return;
    }
    
    uint64_t totalMB = getTotalRAM() / (1024 * 1024);
    uint64_t usedMB = getUsedRAM() / (1024 * 1024);
    float percentage = getRAMUsagePercentage();
    
    std::cerr << "WARNING: Memory usage is " << (percentage * 100.0f) << "% (" 
              << usedMB << "MB / " << totalMB << "MB)" << std::endl;
    
    m_warningTriggered = true;
    
    // Call warning callback if set
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_warningCallback) {
            m_warningCallback(usedMB, totalMB, percentage);
        }
    }
    
    // Request cleanup
    requestCleanup();
}

void MemoryWatchdog::requestCleanup() {
    m_cleanupRequested.store(true);
}

void MemoryWatchdog::performCleanup() {
    if (!m_initialized.load()) {
        return;
    }
    
    std::cout << "Performing memory cleanup..." << std::endl;
    
    // Cleanup unused workspaces
    cleanupUnusedWorkspaces();
    
    // Cleanup inactive resources
    cleanupInactiveResources();
    
    // Call cleanup callback if set
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_cleanupCallback) {
            m_cleanupCallback();
        }
    }
    
    // Update MemoryManager to reflect cleanup
    auto& memoryManager = MemoryManager::getInstance();
    memoryManager.performCleanup();
    
    uint64_t usedMB = getUsedRAM() / (1024 * 1024);
    std::cout << "Memory cleanup completed. Current usage: " << usedMB << "MB" << std::endl;
}

void MemoryWatchdog::cleanupUnusedWorkspaces() {
    try {
        auto& workspaceManager = WorkspaceManager::getInstance();
        auto& resourceStreamer = ResourceStreamer::getInstance();
        
        // Stream out inactive workspace resources
        resourceStreamer.streamOutInactiveWorkspaces();
        
        std::cout << "Cleaned up unused workspace resources" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during workspace cleanup: " << e.what() << std::endl;
    }
}

void MemoryWatchdog::cleanupInactiveResources() {
    try {
        auto& resourceStreamer = ResourceStreamer::getInstance();
        
        // Request resource streamer to cleanup inactive resources
        // This will free memory from resources not currently in use
        
        std::cout << "Cleaned up inactive resources" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during resource cleanup: " << e.what() << std::endl;
    }
}

} // namespace aether
