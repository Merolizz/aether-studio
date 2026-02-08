#include "ImGuiManager.h"
#include "../core/Window.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
#include "../engine/render/DirectX11Renderer.h"
#include "backends/imgui_impl_dx11.h"
#endif
#ifdef AETHER_DIRECTX_12
#include "../engine/render/DirectX12Renderer.h"
#include "backends/imgui_impl_dx12.h"
#endif
#else
#include "../engine/render/VulkanRenderer.h"
#include "backends/imgui_impl_vulkan.h"
#endif

namespace aether {

ImGuiManager::ImGuiManager() {}

ImGuiManager::~ImGuiManager() {
    shutdown();
}

#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
bool ImGuiManager::initialize(Window* window, DirectX11Renderer* renderer) {
    m_window = window;
    m_renderer = renderer;
    m_directX11 = true;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOther(window->getHandle(), true);
    if (!ImGui_ImplDX11_Init(renderer->getDevice(), renderer->getDeviceContext())) {
        std::cerr << "ImGui_ImplDX11_Init failed" << std::endl;
        return false;
    }
    m_initialized = true;
    return true;
}
#endif
#ifdef AETHER_DIRECTX_12
bool ImGuiManager::initialize(Window* window, DirectX12Renderer* renderer) {
    m_window = window;
    m_renderer = renderer;
    m_directX11 = false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOther(window->getHandle(), true);

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = renderer->getDevice();
    init_info.CommandQueue = renderer->getCommandQueue();
    init_info.NumFramesInFlight = renderer->getNumFramesInFlight();
    init_info.RTVFormat = renderer->getRtvFormat();
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
    init_info.SrvDescriptorHeap = renderer->getSrvDescriptorHeap();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu) {
        DirectX12Renderer* r = static_cast<DirectX12Renderer*>(info->UserData);
        r->allocSrvDescriptor(out_cpu, out_gpu);
    };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
        DirectX12Renderer* r = static_cast<DirectX12Renderer*>(info->UserData);
        r->freeSrvDescriptor(cpu_handle, gpu_handle);
    };
    init_info.UserData = renderer;

    if (!ImGui_ImplDX12_Init(&init_info)) {
        std::cerr << "ImGui_ImplDX12_Init failed" << std::endl;
        return false;
    }
    m_initialized = true;
    return true;
}
#endif
#else
bool ImGuiManager::initialize(Window* window, VulkanRenderer* renderer) {
    m_window = window;
    m_renderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window->getHandle(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = renderer->getInstance();
    init_info.PhysicalDevice = renderer->getPhysicalDevice();
    init_info.Device = renderer->getDevice();
    init_info.QueueFamily = 0;
    init_info.Queue = renderer->getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = VK_NULL_HANDLE;
    init_info.MinImageCount = 2;
    init_info.ImageCount = renderer->getImageCount();
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.UseDynamicRendering = false;

    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(renderer->getDevice(), &pool_info, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        std::cerr << "Failed to create descriptor pool for ImGui" << std::endl;
        return false;
    }

    init_info.DescriptorPool = m_descriptorPool;
    init_info.PipelineInfoMain.RenderPass = renderer->getRenderPass();
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);
    m_initialized = true;
    return true;
}
#endif

void ImGuiManager::shutdown() {
    if (!m_initialized) {
        return;
    }

#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
    ImGui_ImplDX11_Shutdown();
#endif
#ifdef AETHER_DIRECTX_12
    ImGui_ImplDX12_Shutdown();
#endif
#else
    if (m_renderer && m_renderer->getDevice() != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_renderer->getDevice());
    }
    ImGui_ImplVulkan_Shutdown();
    if (m_descriptorPool != VK_NULL_HANDLE && m_renderer) {
        vkDestroyDescriptorPool(m_renderer->getDevice(), m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }
#endif

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_renderer = nullptr;
    m_initialized = false;
}

void ImGuiManager::beginFrame() {
    if (!m_initialized) {
        return;
    }

#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
    ImGui_ImplDX11_NewFrame();
#endif
#ifdef AETHER_DIRECTX_12
    ImGui_ImplDX12_NewFrame();
#endif
#else
    ImGui_ImplVulkan_NewFrame();
#endif
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::endFrame() {
    if (!m_initialized) {
        return;
    }
    ImGui::Render();
}

#ifdef AETHER_USE_DIRECTX
void ImGuiManager::render() {
    if (!m_initialized) {
        return;
    }
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data) return;

    if (m_directX11) {
#ifdef AETHER_DIRECTX_11
        ImGui_ImplDX11_RenderDrawData(draw_data);
#endif
    } else {
#ifdef AETHER_DIRECTX_12
        DirectX12Renderer* r = static_cast<DirectX12Renderer*>(m_renderer);
        ImGui_ImplDX12_RenderDrawData(draw_data, r->getCommandList());
#endif
    }
}
#else
void ImGuiManager::render(VkCommandBuffer commandBuffer) {
    if (!m_initialized) {
        return;
    }
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (draw_data) {
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    }
}
#endif

} // namespace aether
