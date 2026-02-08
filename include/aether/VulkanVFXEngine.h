#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace aether {

struct VulkanContext;

/** GPU-accelerated VFX: Blur, Color Correction, Particles via Vulkan Compute Shaders. */
class VulkanVFXEngine {
public:
    VulkanVFXEngine();
    ~VulkanVFXEngine();

    VulkanVFXEngine(const VulkanVFXEngine&) = delete;
    VulkanVFXEngine& operator=(const VulkanVFXEngine&) = delete;

    bool initialize(void* vulkanInstance, void* physicalDevice, void* device);
    void shutdown();

    /** Run blur pass on input texture; output to output texture. */
    void dispatchBlur(uint32_t width, uint32_t height, float radius);
    /** Run color correction (3-way or LUT) pass. */
    void dispatchColorCorrection(uint32_t width, uint32_t height);
    /** Run particle simulation step. */
    void dispatchParticles(uint32_t count, float deltaTime);

    bool isInitialized() const { return m_initialized; }

private:
    bool createComputePipelines();
    bool m_initialized = false;
    void* m_device = nullptr;
    void* m_blurPipeline = nullptr;
    void* m_colorCorrectionPipeline = nullptr;
    void* m_particlesPipeline = nullptr;
};

} // namespace aether
