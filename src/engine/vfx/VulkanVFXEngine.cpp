#include "aether/VulkanVFXEngine.h"

namespace aether {

VulkanVFXEngine::VulkanVFXEngine() = default;

VulkanVFXEngine::~VulkanVFXEngine() {
    shutdown();
}

bool VulkanVFXEngine::initialize(void* vulkanInstance, void* physicalDevice, void* device) {
    if (m_initialized) return true;
    (void)vulkanInstance;
    (void)physicalDevice;
    m_device = device;
    m_initialized = createComputePipelines();
    return m_initialized;
}

void VulkanVFXEngine::shutdown() {
    m_blurPipeline = nullptr;
    m_colorCorrectionPipeline = nullptr;
    m_particlesPipeline = nullptr;
    m_device = nullptr;
    m_initialized = false;
}

void VulkanVFXEngine::dispatchBlur(uint32_t width, uint32_t height, float radius) {
    (void)width;
    (void)height;
    (void)radius;
    if (!m_initialized || !m_blurPipeline) return;
}

void VulkanVFXEngine::dispatchColorCorrection(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    if (!m_initialized || !m_colorCorrectionPipeline) return;
}

void VulkanVFXEngine::dispatchParticles(uint32_t count, float deltaTime) {
    (void)count;
    (void)deltaTime;
    if (!m_initialized || !m_particlesPipeline) return;
}

bool VulkanVFXEngine::createComputePipelines() {
    return true;
}

} // namespace aether
