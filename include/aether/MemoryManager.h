#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

namespace aether {

class MemoryManager {
public:
    using MemoryWarningCallback = std::function<void(uint64_t usedMB, uint64_t totalMB, float percentage)>;
    
    // Singleton access
    static MemoryManager& getInstance();
    
    // Delete copy constructor and assignment operator
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    // Initialization
    bool initialize();
    void shutdown();
    
    // Memory monitoring
    void update();
    uint64_t getTotalRAM() const { return m_totalRAM; }
    uint64_t getUsedRAM() const { return m_usedRAM; }
    uint64_t getAvailableRAM() const { return m_availableRAM; }
    float getRAMUsagePercentage() const;
    
    // Limit management
    void setRAMLimitPercentage(float percentage); // 0.0 to 1.0 (default 0.75 = 75%)
    float getRAMLimitPercentage() const { return m_ramLimitPercentage; }
    bool isOverLimit() const;
    
    // Callbacks
    void setWarningCallback(MemoryWarningCallback callback) { m_warningCallback = callback; }
    
    // Memory cleanup
    void requestCleanup();
    bool performCleanup();

private:
    MemoryManager() = default;
    ~MemoryManager() = default;

    bool querySystemMemory();
    void checkMemoryLimit();

    uint64_t m_totalRAM = 0;        // Total RAM in bytes
    uint64_t m_usedRAM = 0;         // Used RAM in bytes
    uint64_t m_availableRAM = 0;    // Available RAM in bytes
    float m_ramLimitPercentage = 0.75f; // 75% default limit
    
    MemoryWarningCallback m_warningCallback;
    bool m_warningTriggered = false;
    bool m_initialized = false;
    
    mutable std::mutex m_mutex;
};

} // namespace aether
