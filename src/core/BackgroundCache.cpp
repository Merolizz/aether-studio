#include "../../include/aether/BackgroundCache.h"
#include "../../include/aether/HardwareOrchestrator.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <pdh.h>
#include <windows.h>

#pragma comment(lib, "pdh.lib")
#endif

namespace aether {

BackgroundCache &BackgroundCache::getInstance() {
  static BackgroundCache instance;
  return instance;
}

bool BackgroundCache::initialize(const std::string &cacheDirectory) {
  (void)cacheDirectory; // Currently bypassed - will be used when cache thread
                        // is enabled
  std::cout << "Debug: BackgroundCache::initialize start (Bypassed)"
            << std::endl;
  /*
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_initialized) {
      return true;
  }

  m_cacheDirectory = cacheDirectory;
  std::cout << "Debug: Creating cache directory: " << m_cacheDirectory <<
  std::endl;

  // Create cache directory if it doesn't exist
  try {
      std::filesystem::create_directories(m_cacheDirectory);
  } catch (const std::exception& e) {
      std::cerr << "Failed to create cache directory: " << e.what() <<
  std::endl; return false;
  }

  m_shouldStop = false;
  std::cout << "Debug: Starting cache thread" << std::endl;
  m_cacheThread = std::thread(&BackgroundCache::cacheThread, this);
  */

  m_initialized = true; // Mark as initialized to allow shutdown to not crash?
  m_shouldStop = true;  // Ensure thread loop condition is false
  std::cout << "Background Cache initialized: (Bypassed)" << std::endl;
  return true;
}

void BackgroundCache::shutdown() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!m_initialized) {
    return;
  }

  m_shouldStop = true;

  if (m_cacheThread.joinable()) {
    m_cacheThread.join();
  }

  m_initialized = false;
}

void BackgroundCache::cacheThread() {
  std::cout << "Background cache thread started" << std::endl;

  try {
    while (!m_shouldStop) {
      // Check if GPU is idle
      if (m_autoCache && isGPUIdle()) {
        processCacheQueue();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } catch (const std::exception &e) {
    std::cerr << "Background cache thread exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Background cache thread unknown exception" << std::endl;
  }

  std::cout << "Background cache thread stopped" << std::endl;
}

void BackgroundCache::processCacheQueue() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_cacheQueue.empty()) {
    return;
  }

  auto [effectId, parameters] = m_cacheQueue.front();
  m_cacheQueue.pop();

  std::string cachePath = getCacheFilePath(effectId);

  // Render effect to cache
  if (renderEffect(effectId, parameters, cachePath)) {
    CacheEntry entry;
    entry.effectId = effectId;
    entry.cachePath = cachePath;
    entry.isComplete = true;
    entry.createdTime = std::chrono::system_clock::now();

    try {
      entry.fileSize = std::filesystem::file_size(cachePath);
    } catch (...) {
      entry.fileSize = 0;
    }

    m_cacheEntries.push_back(entry);

    if (m_cacheCompletedCallback) {
      m_cacheCompletedCallback(entry);
    }

    std::cout << "Effect cached: " << effectId << " -> " << cachePath
              << std::endl;
  }
}

void BackgroundCache::queueEffect(const std::string &effectId,
                                  const std::string &parameters) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Check if already cached
  if (isEffectCached(effectId)) {
    return;
  }

  // Check if already in queue
  std::queue<std::pair<std::string, std::string>> tempQueue = m_cacheQueue;
  while (!tempQueue.empty()) {
    if (tempQueue.front().first == effectId) {
      return; // Already queued
    }
    tempQueue.pop();
  }

  m_cacheQueue.push({effectId, parameters});
  std::cout << "Effect queued for caching: " << effectId << std::endl;
}

void BackgroundCache::cancelEffect(const std::string &effectId) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Remove from queue
  std::queue<std::pair<std::string, std::string>> newQueue;
  while (!m_cacheQueue.empty()) {
    if (m_cacheQueue.front().first != effectId) {
      newQueue.push(m_cacheQueue.front());
    }
    m_cacheQueue.pop();
  }
  m_cacheQueue = newQueue;
}

bool BackgroundCache::isEffectCached(const std::string &effectId) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  for (const auto &entry : m_cacheEntries) {
    if (entry.effectId == effectId && entry.isComplete) {
      return std::filesystem::exists(entry.cachePath);
    }
  }

  return false;
}

std::string BackgroundCache::getCachePath(const std::string &effectId) const {
  std::lock_guard<std::mutex> lock(m_mutex);

  for (const auto &entry : m_cacheEntries) {
    if (entry.effectId == effectId && entry.isComplete) {
      return entry.cachePath;
    }
  }

  return "";
}

float BackgroundCache::getGPUUtilization() const {
#ifdef _WIN32
  // Try to query GPU utilization via Performance Data Helper (PDH)
  // This is a basic implementation - for production use NVML/ADL
  static HQUERY hQuery = NULL;
  static HCOUNTER hCounter = NULL;
  static bool initialized = false;

  if (!initialized) {
    if (PdhOpenQuery(NULL, 0, &hQuery) == ERROR_SUCCESS) {
      // Try to add GPU utilization counter (this may not work on all systems)
      // For NVIDIA: "\\GPU Engine(*)\\Utilization Percentage"
      // For AMD: "\\GPU Engine(*)\\Utilization Percentage"
      // This is a simplified version
      PDH_STATUS status = PdhAddCounterA(
          hQuery, "\\GPU Engine(*)\\Utilization Percentage", 0, &hCounter);

      if (status == ERROR_SUCCESS) {
        initialized = true;
      } else {
        PdhCloseQuery(hQuery);
        hQuery = NULL;
      }
    }
  }

  if (initialized && hQuery && hCounter) {
    PdhCollectQueryData(hQuery);

    PDH_FMT_COUNTERVALUE value;
    if (PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &value) ==
        ERROR_SUCCESS) {
      return static_cast<float>(value.doubleValue);
    }
  }

  // Fallback: Check if GPU is being used by checking render mode
  auto &hardwareOrchestrator = HardwareOrchestrator::getInstance();
  auto renderMode = hardwareOrchestrator.getCurrentRenderMode();

  // If using software rendering, GPU is likely idle
  if (renderMode == RenderMode::Software) {
    return 0.0f;
  }

  // Otherwise, assume some usage (conservative estimate)
  return 10.0f;
#else
  // Linux: Would use nvidia-smi or similar
  // For now, return a conservative estimate
  return 10.0f;
#endif
}

bool BackgroundCache::isGPUIdle() const {
  return getGPUUtilization() < m_gpuIdleThreshold;
}

size_t BackgroundCache::getQueueSize() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_cacheQueue.size();
}

size_t BackgroundCache::getCacheSize() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_cacheEntries.size();
}

uint64_t BackgroundCache::getTotalCacheSize() const {
  std::lock_guard<std::mutex> lock(m_mutex);

  uint64_t total = 0;
  for (const auto &entry : m_cacheEntries) {
    total += entry.fileSize;
  }

  return total;
}

bool BackgroundCache::renderEffect(const std::string &effectId,
                                   const std::string &parameters,
                                   const std::string &outputPath) {
  // Placeholder for effect rendering
  // In full implementation, this would:
  // 1. Load effect shader/parameters
  // 2. Render effect to texture
  // 3. Save to disk (SSD)

  // For now, just create a placeholder file
  std::ofstream file(outputPath);
  if (file.is_open()) {
    file << "Cached effect: " << effectId << "\n";
    file << "Parameters: " << parameters << "\n";
    file.close();
    return true;
  }

  return false;
}

std::string
BackgroundCache::getCacheFilePath(const std::string &effectId) const {
  std::string filename = effectId + ".cache";
  return (std::filesystem::path(m_cacheDirectory) / filename).string();
}

} // namespace aether
