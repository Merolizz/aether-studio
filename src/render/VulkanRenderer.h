#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace aether {
class Window;
}

namespace aether {

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    bool initialize(Window* window);
    void shutdown();

    bool beginFrame();
    void beginRenderPass();
    void endRenderPass();
    void endFrame();

    VkInstance getInstance() const { return m_instance; }
    VkDevice getDevice() const { return m_device; }
    VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
    VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
    VkQueue getPresentQueue() const { return m_presentQueue; }
    VkCommandPool getCommandPool() const { return m_commandPool; }
    VkRenderPass getRenderPass() const { return m_renderPass; }
    VkFormat getSwapchainImageFormat() const { return m_swapchainImageFormat; }
    VkExtent2D getSwapchainExtent() const { return m_swapchainExtent; }
    uint32_t getImageCount() const { return static_cast<uint32_t>(m_swapchainImages.size()); }
    VkImageView getSwapchainImageView(uint32_t index) const { return m_swapchainImageViews[index]; }
    VkFramebuffer getFramebuffer(uint32_t index) const { return m_swapchainFramebuffers[index]; }
    VkCommandBuffer getCurrentCommandBuffer() const { return m_commandBuffers[m_currentImageIndex]; }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

private:
    bool createInstance();
    bool createSurface(Window* window);
    bool selectPhysicalDevice();
    bool createLogicalDevice();
    bool createSwapchain(Window* window);
    void destroySwapchain();
    bool createImageViews();
    bool createRenderPass();
    bool createFramebuffers();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapchainFramebuffers;
    VkFormat m_swapchainImageFormat;
    VkExtent2D m_swapchainExtent;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    size_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;

    bool m_framebufferResized = false;
    Window* m_window = nullptr;
};

} // namespace aether
