#pragma once

#include <memory>

#ifdef AETHER_USE_DIRECTX
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D12GraphicsCommandList;
#else
#include <vulkan/vulkan.h>
#endif

namespace aether {
class Window;
class VulkanRenderer;
#ifdef AETHER_DIRECTX_11
class DirectX11Renderer;
#endif
#ifdef AETHER_DIRECTX_12
class DirectX12Renderer;
#endif
}

namespace aether {

class ImGuiManager {
public:
    ImGuiManager();
    ~ImGuiManager();

    ImGuiManager(const ImGuiManager&) = delete;
    ImGuiManager& operator=(const ImGuiManager&) = delete;

#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
    bool initialize(Window* window, DirectX11Renderer* renderer);
#endif
#ifdef AETHER_DIRECTX_12
    bool initialize(Window* window, DirectX12Renderer* renderer);
#endif
    void render();
#else
    bool initialize(Window* window, VulkanRenderer* renderer);
    void render(VkCommandBuffer commandBuffer);
#endif

    void shutdown();
    void beginFrame();
    void endFrame();

private:
    Window* m_window = nullptr;
#ifdef AETHER_USE_DIRECTX
    void* m_renderer = nullptr;
    bool m_directX11 = false;
#else
    VulkanRenderer* m_renderer = nullptr;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
#endif
    bool m_initialized = false;
};

} // namespace aether
