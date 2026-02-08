#pragma once

#if defined(_WIN32) && defined(AETHER_DIRECTX_12)

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

namespace aether {
class Window;
}

namespace aether {

static constexpr UINT NUM_FRAMES_IN_FLIGHT = 2;
static constexpr UINT NUM_BACK_BUFFERS = 2;
static constexpr UINT SRV_HEAP_SIZE = 64;

class DirectX12Renderer {
public:
    DirectX12Renderer();
    ~DirectX12Renderer();

    DirectX12Renderer(const DirectX12Renderer&) = delete;
    DirectX12Renderer& operator=(const DirectX12Renderer&) = delete;

    bool initialize(Window* window);
    void shutdown();

    bool beginFrame();
    void beginRenderPass();
    void endRenderPass();
    void endFrame();

    ID3D12Device* getDevice() const { return m_device.Get(); }
    ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue.Get(); }
    ID3D12GraphicsCommandList* getCommandList() const { return m_commandList.Get(); }
    ID3D12DescriptorHeap* getRtvDescriptorHeap() const { return m_rtvDescriptorHeap.Get(); }
    ID3D12DescriptorHeap* getSrvDescriptorHeap() const { return m_srvDescriptorHeap.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRtvDescriptor() const { return m_mainRenderTargetDescriptor[m_frameIndex]; }
    DXGI_FORMAT getRtvFormat() const { return DXGI_FORMAT_R8G8B8A8_UNORM; }
    UINT getNumFramesInFlight() const { return NUM_FRAMES_IN_FLIGHT; }
    UINT getCurrentBackBufferIndex() const { return m_backBufferIndex; }

    // SRV descriptor allocator for ImGui (simple linear allocator)
    bool allocSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu);
    void freeSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

private:
    bool createDeviceAndSwapChain(Window* window);
    bool createRenderTarget();
    void cleanupRenderTarget();
    void waitForPreviousFrame();

    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[NUM_FRAMES_IN_FLIGHT];
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    UINT64 m_fenceValues[NUM_FRAMES_IN_FLIGHT] = {};
    UINT64 m_fenceLastSignaledValue = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescriptorHeap;
    UINT m_rtvDescriptorSize = 0;
    UINT m_srvDescriptorSize = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> m_mainRenderTargetResource[NUM_BACK_BUFFERS];
    D3D12_CPU_DESCRIPTOR_HANDLE m_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

    UINT m_frameIndex = 0;
    UINT m_backBufferIndex = 0;
    Window* m_window = nullptr;

    // Simple SRV heap allocator state
    std::vector<int> m_srvFreeIndices;
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvHeapStartCpu = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvHeapStartGpu = {};
};

} // namespace aether

#endif // _WIN32 && AETHER_DIRECTX_12
