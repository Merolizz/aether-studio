#include "../../include/aether/MemoryManager.h"
#include "../../include/aether/ResourceStreamer.h"
#include "../../include/aether/BackgroundCache.h"
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#endif
#include <iostream>

namespace aether {

MemoryManager& MemoryManager::getInstance() {
    static MemoryManager instance;
    return instance;
}

bool MemoryManager::initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    if (!querySystemMemory()) {
        std::cerr << "Failed to query system memory" << std::endl;
        return false;
    }
    
    m_initialized = true;
    std::cout << "Memory Manager initialized - Total RAM: " << (m_totalRAM / (1024 * 1024)) << " MB" << std::endl;
    return true;
}

void MemoryManager::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_initialized = false;
}

void MemoryManager::update() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    querySystemMemory();
    checkMemoryLimit();
}

bool MemoryManager::querySystemMemory() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        m_totalRAM = memInfo.ullTotalPhys;
        m_availableRAM = memInfo.ullAvailPhys;
        m_usedRAM = m_totalRAM - m_availableRAM;
        return true;
    }
    
    return false;
#else
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        m_totalRAM = si.totalram * si.mem_unit;
        m_availableRAM = si.freeram * si.mem_unit;
        m_usedRAM = m_totalRAM - m_availableRAM;
        return true;
    }
    
    return false;
#endif
}

float MemoryManager::getRAMUsagePercentage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_totalRAM == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_usedRAM) / static_cast<float>(m_totalRAM) * 100.0f;
}

void MemoryManager::setRAMLimitPercentage(float percentage) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (percentage < 0.0f || percentage > 1.0f) {
        std::cerr << "Invalid RAM limit percentage: " << percentage << " (must be 0.0-1.0)" << std::endl;
        return;
    }
    
    m_ramLimitPercentage = percentage;
    std::cout << "RAM limit set to: " << (percentage * 100.0f) << "%" << std::endl;
}

bool MemoryManager::isOverLimit() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_totalRAM == 0) {
        return false;
    }
    
    float usagePercentage = static_cast<float>(m_usedRAM) / static_cast<float>(m_totalRAM);
    return usagePercentage > m_ramLimitPercentage;
}

void MemoryManager::checkMemoryLimit() {
    if (isOverLimit()) {
        if (!m_warningTriggered) {
            m_warningTriggered = true;
            
            uint64_t usedMB = m_usedRAM / (1024 * 1024);
            uint64_t totalMB = m_totalRAM / (1024 * 1024);
            float percentage = getRAMUsagePercentage();
            
            std::cerr << "WARNING: RAM usage exceeded limit! Used: " << usedMB 
                      << " MB / " << totalMB << " MB (" << percentage << "%)" << std::endl;
            
            if (m_warningCallback) {
                m_warningCallback(usedMB, totalMB, percentage);
            }
            
            // Request cleanup
            requestCleanup();
        }
    } else {
        m_warningTriggered = false;
    }
}

void MemoryManager::requestCleanup() {
    // This will trigger cleanup in the next frame
    // Actual cleanup is handled by performCleanup()
}

bool MemoryManager::performCleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isOverLimit()) {
        return false; // No cleanup needed
    }
    
    std::cout << "Performing memory cleanup..." << std::endl;
    
    // 1. Stream out inactive workspace resources
    try {
        auto& resourceStreamer = ResourceStreamer::getInstance();
        resourceStreamer.streamOutInactiveWorkspaces();
        std::cout << "  - Streamed out inactive workspaces" << std::endl;
    } catch (...) {
        std::cerr << "  - Failed to stream out workspaces" << std::endl;
    }
    
    // 2. Clear background cache if too large
    try {
        auto& backgroundCache = BackgroundCache::getInstance();
        uint64_t cacheSize = backgroundCache.getTotalCacheSize();
        if (cacheSize > 1024 * 1024 * 1024) { // 1GB limit
            // Cancel pending cache operations
            // Note: Actual cache file deletion would require more implementation
            std::cout << "  - Background cache size: " << (cacheSize / (1024 * 1024)) << " MB" << std::endl;
        }
    } catch (...) {
        std::cerr << "  - Failed to check background cache" << std::endl;
    }
    
    // 3. Force system memory cleanup (Windows specific)
#ifdef _WIN32
    // Suggest to OS to trim working sets
    SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1));
#endif
    
    // Re-query memory after cleanup
    querySystemMemory();
    
    float newPercentage = getRAMUsagePercentage();
    std::cout << "Memory cleanup completed. New usage: " << newPercentage << "%" << std::endl;
    
    return true;
}

} // namespace aether
