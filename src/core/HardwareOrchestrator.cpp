#include "../../include/aether/HardwareOrchestrator.h"
#include <algorithm>
#include <iostream>
#include <cstring>

namespace aether {

HardwareOrchestrator& HardwareOrchestrator::getInstance() {
    static HardwareOrchestrator instance;
    return instance;
}

bool HardwareOrchestrator::initialize(VkInstance vulkanInstance) {
    if (vulkanInstance == VK_NULL_HANDLE) {
        std::cerr << "Invalid Vulkan instance" << std::endl;
        return false;
    }
    
    m_vulkanInstance = vulkanInstance;
    
    if (!detectGPUs()) {
        std::cerr << "Failed to detect GPUs" << std::endl;
        return false;
    }
    
    // Select primary GPU
    if (!m_availableGPUs.empty()) {
        selectGPU(0);
        m_currentRenderMode = selectOptimalRenderMode();
    } else {
        // Fallback to software rendering
        m_currentRenderMode = RenderMode::Software;
        std::cerr << "No GPUs detected, falling back to software rendering" << std::endl;
    }
    
    m_initialized = true;
    return true;
}

void HardwareOrchestrator::shutdown() {
    m_availableGPUs.clear();
    m_primaryGPU = GPUInfo{};
    m_vulkanInstance = VK_NULL_HANDLE;
    m_initialized = false;
}

bool HardwareOrchestrator::detectGPUs() {
    if (m_vulkanInstance == VK_NULL_HANDLE) {
        return false;
    }
    
    m_availableGPUs.clear();
    
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        std::cerr << "No Vulkan-capable devices found" << std::endl;
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        GPUInfo gpuInfo;
        gpuInfo.vulkanDevice = device;
        
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        
        gpuInfo.name = properties.deviceName;
        gpuInfo.deviceId = properties.deviceID;
        
        // Driver version
        uint32_t major = VK_VERSION_MAJOR(properties.driverVersion);
        uint32_t minor = VK_VERSION_MINOR(properties.driverVersion);
        uint32_t patch = VK_VERSION_PATCH(properties.driverVersion);
        gpuInfo.driverVersion = std::to_string(major) + "." + 
                               std::to_string(minor) + "." + 
                               std::to_string(patch);
        
        // Memory information
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);
        
        uint64_t totalMemory = 0;
        for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
            if (memoryProperties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalMemory += memoryProperties.memoryHeaps[i].size;
            }
        }
        gpuInfo.totalMemory = totalMemory;
        gpuInfo.availableMemory = totalMemory; // Simplified
        
        // Identify vendor
        gpuInfo.vendor = identifyVendor(gpuInfo.name);
        
        // Detect capabilities based on vendor
        switch (gpuInfo.vendor) {
            case GPUVendor::NVIDIA:
                detectNVIDIAGPU(gpuInfo);
                break;
            case GPUVendor::AMD:
                detectAMDGPU(gpuInfo);
                break;
            case GPUVendor::Intel:
                detectIntelGPU(gpuInfo);
                break;
            default:
                break;
        }
        
        // Check for dedicated GPU (not integrated)
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_hasDedicatedGPU = true;
        }
        
        m_availableGPUs.push_back(gpuInfo);
        
        std::cout << "Detected GPU: " << gpuInfo.name 
                  << " (Vendor: " << static_cast<int>(gpuInfo.vendor) << ")" << std::endl;
    }
    
    return !m_availableGPUs.empty();
}

GPUVendor HardwareOrchestrator::identifyVendor(const std::string& deviceName) {
    std::string lowerName = deviceName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName.find("nvidia") != std::string::npos || 
        lowerName.find("geforce") != std::string::npos ||
        lowerName.find("quadro") != std::string::npos ||
        lowerName.find("rtx") != std::string::npos ||
        lowerName.find("gtx") != std::string::npos) {
        return GPUVendor::NVIDIA;
    }
    
    if (lowerName.find("amd") != std::string::npos ||
        lowerName.find("radeon") != std::string::npos ||
        lowerName.find("rx") != std::string::npos) {
        return GPUVendor::AMD;
    }
    
    if (lowerName.find("intel") != std::string::npos ||
        lowerName.find("iris") != std::string::npos ||
        lowerName.find("uhd") != std::string::npos) {
        return GPUVendor::Intel;
    }
    
    return GPUVendor::Unknown;
}

void HardwareOrchestrator::detectNVIDIAGPU(const GPUInfo& gpuInfo) {
    GPUInfo& gpu = const_cast<GPUInfo&>(gpuInfo);
    
    // Check for CUDA support (simplified - would need CUDA runtime)
    // For RTX series, assume CUDA support
    std::string lowerName = gpu.name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName.find("rtx") != std::string::npos ||
        lowerName.find("gtx") != std::string::npos ||
        lowerName.find("quadro") != std::string::npos) {
        gpu.supportsCUDA = true;
        gpu.supportsNVENC = true; // Most modern NVIDIA GPUs support NVENC
    }
    
    // TODO: More detailed CUDA capability detection
}

void HardwareOrchestrator::detectAMDGPU(const GPUInfo& gpuInfo) {
    GPUInfo& gpu = const_cast<GPUInfo&>(gpuInfo);
    
    // AMD GPUs typically support OpenCL
    gpu.supportsOpenCL = true;
    
    // Check for VCE (Video Coding Engine) support
    // Modern AMD GPUs (RX series) support VCE
    std::string lowerName = gpu.name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    if (lowerName.find("rx") != std::string::npos ||
        lowerName.find("radeon") != std::string::npos) {
        gpu.supportsVCE = true;
    }
}

void HardwareOrchestrator::detectIntelGPU(const GPUInfo& gpuInfo) {
    GPUInfo& gpu = const_cast<GPUInfo&>(gpuInfo);
    
    // Intel integrated GPUs support QuickSync
    gpu.supportsQuickSync = true;
    
    // Intel GPUs may support OpenCL
    gpu.supportsOpenCL = true;
}

bool HardwareOrchestrator::selectGPU(uint32_t index) {
    if (index >= m_availableGPUs.size()) {
        std::cerr << "Invalid GPU index: " << index << std::endl;
        return false;
    }
    
    m_selectedGPUIndex = index;
    m_primaryGPU = m_availableGPUs[index];
    m_currentRenderMode = selectOptimalRenderMode();
    
    std::cout << "Selected GPU: " << m_primaryGPU.name << std::endl;
    std::cout << "Render mode: " << static_cast<int>(m_currentRenderMode) << std::endl;
    
    return true;
}

const GPUInfo& HardwareOrchestrator::getSelectedGPU() const {
    if (m_selectedGPUIndex < m_availableGPUs.size()) {
        return m_availableGPUs[m_selectedGPUIndex];
    }
    return m_primaryGPU;
}

RenderMode HardwareOrchestrator::selectOptimalRenderMode() const {
    if (m_availableGPUs.empty()) {
        return RenderMode::Software;
    }
    
    const GPUInfo& gpu = getSelectedGPU();
    return determineBestRenderMode(gpu);
}

RenderMode HardwareOrchestrator::determineBestRenderMode(const GPUInfo& gpu) const {
    // Priority order:
    // 1. CUDA (NVIDIA RTX/GTX)
    // 2. Vulkan (Universal, best performance)
    // 3. OpenCL (AMD/Intel)
    // 4. QuickSync (Intel iGPU)
    // 5. Software (Fallback)
    
    if (gpu.supportsCUDA && gpu.vendor == GPUVendor::NVIDIA) {
        // CUDA is preferred for NVIDIA GPUs with compute workloads
        // But for rendering, Vulkan is still better
        // So we'll use Vulkan as default, CUDA can be used for specific compute tasks
        return RenderMode::Vulkan;
    }
    
    if (gpu.supportsVulkan) {
        return RenderMode::Vulkan;
    }
    
    if (gpu.supportsOpenCL) {
        return RenderMode::OpenCL;
    }
    
    if (gpu.supportsQuickSync && gpu.vendor == GPUVendor::Intel) {
        return RenderMode::QuickSync;
    }
    
    return RenderMode::Software;
}

void HardwareOrchestrator::setRenderMode(RenderMode mode) {
    m_currentRenderMode = mode;
    std::cout << "Render mode set to: " << static_cast<int>(mode) << std::endl;
}

bool HardwareOrchestrator::supportsHardwareEncoding() const {
    const GPUInfo& gpu = getSelectedGPU();
    return gpu.supportsNVENC || gpu.supportsVCE || gpu.supportsQuickSync;
}

bool HardwareOrchestrator::supportsHardwareDecoding() const {
    // Most modern GPUs support hardware decoding via Vulkan
    return m_currentRenderMode != RenderMode::Software;
}

bool HardwareOrchestrator::canUseCUDA() const {
    const GPUInfo& gpu = getSelectedGPU();
    return gpu.supportsCUDA && gpu.vendor == GPUVendor::NVIDIA;
}

bool HardwareOrchestrator::canUseOpenCL() const {
    const GPUInfo& gpu = getSelectedGPU();
    return gpu.supportsOpenCL;
}

bool HardwareOrchestrator::canUseQuickSync() const {
    const GPUInfo& gpu = getSelectedGPU();
    return gpu.supportsQuickSync && gpu.vendor == GPUVendor::Intel;
}

} // namespace aether
