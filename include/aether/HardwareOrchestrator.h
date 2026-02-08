#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>

namespace aether {

enum class GPUVendor {
    Unknown,
    NVIDIA,
    AMD,
    Intel,
    Other
};

enum class RenderMode {
    Vulkan,         // Standard Vulkan rendering
    DirectX11,      // DirectX 11 (Windows)
    DirectX12,      // DirectX 12 (Windows)
    CUDA,           // NVIDIA CUDA acceleration
    OpenCL,         // AMD/Intel OpenCL
    QuickSync,      // Intel QuickSync
    Software        // CPU fallback
};

struct GPUInfo {
    GPUVendor vendor = GPUVendor::Unknown;
    std::string name;
    std::string driverVersion;
    uint32_t deviceId = 0;
    uint64_t totalMemory = 0;
    uint64_t availableMemory = 0;
    
    // Capabilities
    bool supportsCUDA = false;
    bool supportsOpenCL = false;
    bool supportsQuickSync = false;
    bool supportsVulkan = true; // Assumed if detected via Vulkan
    bool supportsNVENC = false;  // NVIDIA hardware encoding
    bool supportsVCE = false;    // AMD hardware encoding
    
    VkPhysicalDevice vulkanDevice = VK_NULL_HANDLE;
};

class HardwareOrchestrator {
public:
    // Singleton access
    static HardwareOrchestrator& getInstance();
    
    // Delete copy constructor and assignment operator
    HardwareOrchestrator(const HardwareOrchestrator&) = delete;
    HardwareOrchestrator& operator=(const HardwareOrchestrator&) = delete;

    // Initialization
    bool initialize(VkInstance vulkanInstance);
    void shutdown();

    // GPU Detection
    bool detectGPUs();
    const std::vector<GPUInfo>& getAvailableGPUs() const { return m_availableGPUs; }
    const GPUInfo& getPrimaryGPU() const { return m_primaryGPU; }
    bool hasDedicatedGPU() const { return m_hasDedicatedGPU; }
    
    // Render Mode Selection
    RenderMode selectOptimalRenderMode() const;
    RenderMode getCurrentRenderMode() const { return m_currentRenderMode; }
    void setRenderMode(RenderMode mode);
    
    // GPU Selection
    bool selectGPU(uint32_t index);
    uint32_t getSelectedGPUIndex() const { return m_selectedGPUIndex; }
    const GPUInfo& getSelectedGPU() const;
    
    // Capability Checks
    bool supportsHardwareEncoding() const;
    bool supportsHardwareDecoding() const;
    bool canUseCUDA() const;
    bool canUseOpenCL() const;
    bool canUseQuickSync() const;

private:
    HardwareOrchestrator() = default;
    ~HardwareOrchestrator() = default;

    void detectNVIDIAGPU(const GPUInfo& gpuInfo);
    void detectAMDGPU(const GPUInfo& gpuInfo);
    void detectIntelGPU(const GPUInfo& gpuInfo);
    GPUVendor identifyVendor(const std::string& deviceName);
    RenderMode determineBestRenderMode(const GPUInfo& gpu) const;

    VkInstance m_vulkanInstance = VK_NULL_HANDLE;
    std::vector<GPUInfo> m_availableGPUs;
    GPUInfo m_primaryGPU;
    uint32_t m_selectedGPUIndex = 0;
    RenderMode m_currentRenderMode = RenderMode::Vulkan;
    bool m_hasDedicatedGPU = false;
    bool m_initialized = false;
};

} // namespace aether
