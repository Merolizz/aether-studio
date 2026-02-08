#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <functional>
#include <chrono>

namespace aether {

struct CacheEntry {
    std::string effectId;
    std::string cachePath;
    uint64_t fileSize = 0;
    bool isComplete = false;
    std::chrono::system_clock::time_point createdTime;
};

class BackgroundCache {
public:
    using CacheCompletedCallback = std::function<void(const CacheEntry& entry)>;
    
    // Singleton access
    static BackgroundCache& getInstance();
    
    // Delete copy constructor and assignment operator
    BackgroundCache(const BackgroundCache&) = delete;
    BackgroundCache& operator=(const BackgroundCache&) = delete;

    // Initialization
    bool initialize(const std::string& cacheDirectory);
    void shutdown();

    // Cache management
    void queueEffect(const std::string& effectId, const std::string& parameters);
    void cancelEffect(const std::string& effectId);
    bool isEffectCached(const std::string& effectId) const;
    std::string getCachePath(const std::string& effectId) const;
    
    // GPU monitoring
    float getGPUUtilization() const;
    bool isGPUIdle() const;
    void setGPUIdleThreshold(float threshold) { m_gpuIdleThreshold = threshold; }
    
    // Statistics
    size_t getQueueSize() const;
    size_t getCacheSize() const;
    uint64_t getTotalCacheSize() const; // in bytes
    
    // Callbacks
    void setCacheCompletedCallback(CacheCompletedCallback callback) { m_cacheCompletedCallback = callback; }
    
    // Settings
    void setAutoCache(bool enable) { m_autoCache = enable; }
    bool isAutoCache() const { return m_autoCache; }

private:
    BackgroundCache() = default;
    ~BackgroundCache() = default;

    void cacheThread();
    void processCacheQueue();
    bool renderEffect(const std::string& effectId, const std::string& parameters, const std::string& outputPath);
    std::string getCacheFilePath(const std::string& effectId) const;

    std::string m_cacheDirectory;
    std::queue<std::pair<std::string, std::string>> m_cacheQueue; // effectId, parameters
    std::vector<CacheEntry> m_cacheEntries;
    
    std::atomic<bool> m_shouldStop{false};
    std::thread m_cacheThread;
    
    CacheCompletedCallback m_cacheCompletedCallback;
    bool m_autoCache = true;
    float m_gpuIdleThreshold = 20.0f; // 20% utilization considered idle
    
    mutable std::mutex m_mutex;
    bool m_initialized = false;
};

} // namespace aether
