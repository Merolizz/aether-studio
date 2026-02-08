#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <vulkan/vulkan.h>

namespace aether {
class VulkanRenderer;
class Window;
}

namespace aether {

class SplashScreen {
public:
    SplashScreen();
    ~SplashScreen();

    SplashScreen(const SplashScreen&) = delete;
    SplashScreen& operator=(const SplashScreen&) = delete;

    // Initialization
    bool initialize(VulkanRenderer* renderer, Window* window);
    void shutdown();

    // Display control
    void show();
    void hide();
    bool isVisible() const { return m_visible; }
    bool shouldClose() const; // Returns true after 3 seconds

    // Rendering
    void render();

private:
    bool loadSplashImage(const std::string& imagePath);
    void cleanupVulkanResources();

    VulkanRenderer* m_renderer = nullptr;
    Window* m_window = nullptr;
    
    bool m_initialized = false;
    bool m_visible = false;
    bool m_imageLoaded = false;
    
    // Splash screen timing
    std::chrono::steady_clock::time_point m_startTime;
    static constexpr double SPLASH_DURATION_SECONDS = 3.0;
    static constexpr const char* SPLASH_IMAGE_PATH = "assets/splash.png";
    
    // Image dimensions
    int m_imageWidth = 0;
    int m_imageHeight = 0;

    // Vulkan Resources
    VkImage m_splashImage = VK_NULL_HANDLE;
    VkDeviceMemory m_splashImageMemory = VK_NULL_HANDLE;
    VkImageView m_splashImageView = VK_NULL_HANDLE;
    VkSampler m_splashSampler = VK_NULL_HANDLE;
    VkDescriptorSet m_splashDS = VK_NULL_HANDLE;
};

} // namespace aether
