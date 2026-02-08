#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace aether {

enum class ProxyQuality {
    Low,      // 1/4 resolution
    Medium,   // 1/2 resolution
    High      // 3/4 resolution
};

struct ProxySettings {
    ProxyQuality quality = ProxyQuality::Medium;
    uint32_t targetWidth = 0;
    uint32_t targetHeight = 0;
    std::string codec = "h264";
    uint32_t bitrate = 0; // 0 = auto
    std::string outputPath;
};

class ProxyManager {
public:
    // Singleton access
    static ProxyManager& getInstance();
    
    // Delete copy constructor and assignment operator
    ProxyManager(const ProxyManager&) = delete;
    ProxyManager& operator=(const ProxyManager&) = delete;

    // Initialization
    bool initialize();
    void shutdown();

    // Proxy generation
    bool createProxy(const std::string& sourcePath, const ProxySettings& settings);
    bool isProxyAvailable(const std::string& sourcePath, const ProxySettings& settings) const;
    std::string getProxyPath(const std::string& sourcePath, const ProxySettings& settings) const;
    
    // AI Upscale
    bool canUseAIUpscale() const;
    bool upscaleWithAI(const std::string& sourcePath, uint32_t targetWidth, uint32_t targetHeight, const std::string& outputPath);
    
    // Settings
    void setProxyDirectory(const std::string& directory) { m_proxyDirectory = directory; }
    std::string getProxyDirectory() const { return m_proxyDirectory; }

private:
    ProxyManager() = default;
    ~ProxyManager() = default;

    bool checkTensorCoreSupport() const;
    std::string generateProxyPath(const std::string& sourcePath, const ProxySettings& settings) const;
    bool encodeProxy(const std::string& sourcePath, const ProxySettings& settings);

    std::string m_proxyDirectory = "./cache/proxies";
    bool m_hasTensorCoreSupport = false;
    bool m_initialized = false;
};

} // namespace aether
