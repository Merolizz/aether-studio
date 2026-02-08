#if defined(_WIN32) && defined(AETHER_DIRECTX_12)

#include "DirectX12Renderer.h"
#include "../../core/Window.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace aether {

DirectX12Renderer::DirectX12Renderer() {}

DirectX12Renderer::~DirectX12Renderer() {
    shutdown();
}

bool DirectX12Renderer::initialize(Window* window) {
    m_window = window;
    return createDeviceAndSwapChain(window);
}

void DirectX12Renderer::shutdown() {
    waitForPreviousFrame();
    cleanupRenderTarget();
    if (m_fenceEvent) {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        m_commandAllocators[i].Reset();
    }
    m_commandList.Reset();
    m_commandQueue.Reset();
    m_swapChain.Reset();
    m_rtvDescriptorHeap.Reset();
    m_srvDescriptorHeap.Reset();
    m_fence.Reset();
    m_device.Reset();
    m_window = nullptr;
}

bool DirectX12Renderer::createDeviceAndSwapChain(Window* window) {
    HWND hwnd = glfwGetWin32Window(window->getHandle());
    if (!hwnd) {
        std::cerr << "Failed to get HWND from GLFW window" << std::endl;
        return false;
    }

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    ID3D12Device* pDevice = nullptr;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&pDevice)) != S_OK) {
        std::cerr << "D3D12CreateDevice failed" << std::endl;
        return false;
    }
    m_device.Attach(pDevice);

    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = NUM_BACK_BUFFERS;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvDesc.NodeMask = 1;
    if (m_device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)) != S_OK) {
        std::cerr << "CreateDescriptorHeap RTV failed" << std::endl;
        return false;
    }
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        m_mainRenderTargetDescriptor[i] = rtvHandle;
        rtvHandle.ptr += m_rtvDescriptorSize;
    }

    D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
    srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.NumDescriptors = SRV_HEAP_SIZE;
    srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (m_device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_srvDescriptorHeap)) != S_OK) {
        std::cerr << "CreateDescriptorHeap SRV failed" << std::endl;
        return false;
    }
    m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    m_srvHeapStartCpu = m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_srvHeapStartGpu = m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    m_srvFreeIndices.reserve(SRV_HEAP_SIZE);
    for (int n = SRV_HEAP_SIZE - 1; n >= 0; n--)
        m_srvFreeIndices.push_back(n);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 1;
    ID3D12CommandQueue* pQueue = nullptr;
    if (m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pQueue)) != S_OK) {
        std::cerr << "CreateCommandQueue failed" << std::endl;
        return false;
    }
    m_commandQueue.Attach(pQueue);

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        ID3D12CommandAllocator* pAlloc = nullptr;
        if (m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pAlloc)) != S_OK) {
            std::cerr << "CreateCommandAllocator failed" << std::endl;
            return false;
        }
        m_commandAllocators[i].Attach(pAlloc);
    }

    ID3D12GraphicsCommandList* pList = nullptr;
    if (m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&pList)) != S_OK ||
        pList->Close() != S_OK) {
        std::cerr << "CreateCommandList failed" << std::endl;
        return false;
    }
    m_commandList.Attach(pList);

    ID3D12Fence* pFence = nullptr;
    if (m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)) != S_OK) {
        std::cerr << "CreateFence failed" << std::endl;
        return false;
    }
    m_fence.Attach(pFence);
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent) {
        std::cerr << "CreateEvent failed" << std::endl;
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.BufferCount = NUM_BACK_BUFFERS;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;
    sd.Stereo = FALSE;

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (CreateDXGIFactory1(IID_PPV_ARGS(&factory)) != S_OK) {
        std::cerr << "CreateDXGIFactory1 failed" << std::endl;
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
    if (factory->CreateSwapChainForHwnd(m_commandQueue.Get(), hwnd, &sd, nullptr, nullptr, &swapChain1) != S_OK) {
        std::cerr << "CreateSwapChainForHwnd failed" << std::endl;
        return false;
    }
    if (swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)) != S_OK) {
        std::cerr << "QueryInterface SwapChain3 failed" << std::endl;
        return false;
    }
    m_swapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

    return createRenderTarget();
}

bool DirectX12Renderer::createRenderTarget() {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        ID3D12Resource* pBackBuffer = nullptr;
        if (m_swapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer)) != S_OK) {
            std::cerr << "GetBuffer failed" << std::endl;
            return false;
        }
        m_device->CreateRenderTargetView(pBackBuffer, nullptr, m_mainRenderTargetDescriptor[i]);
        m_mainRenderTargetResource[i].Attach(pBackBuffer);
    }
    return true;
}

void DirectX12Renderer::cleanupRenderTarget() {
    waitForPreviousFrame();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        m_mainRenderTargetResource[i].Reset();
    }
}

void DirectX12Renderer::waitForPreviousFrame() {
    if (!m_fence || !m_fenceEvent) return;

    UINT64 fenceVal = m_fenceValues[m_frameIndex];
    m_commandQueue->Signal(m_fence.Get(), fenceVal);

    if (m_fence->GetCompletedValue() < fenceVal) {
        m_fence->SetEventOnCompletion(fenceVal, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = (m_frameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
}

bool DirectX12Renderer::allocSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) {
    if (m_srvFreeIndices.empty()) return false;
    int idx = m_srvFreeIndices.back();
    m_srvFreeIndices.pop_back();
    outCpu->ptr = m_srvHeapStartCpu.ptr + (SIZE_T)idx * m_srvDescriptorSize;
    outGpu->ptr = m_srvHeapStartGpu.ptr + (SIZE_T)idx * m_srvDescriptorSize;
    return true;
}

void DirectX12Renderer::freeSrvDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE) {
    int idx = (int)((cpuHandle.ptr - m_srvHeapStartCpu.ptr) / m_srvDescriptorSize);
    m_srvFreeIndices.push_back(idx);
}

bool DirectX12Renderer::beginFrame() {
    waitForPreviousFrame();
    m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    if (m_commandAllocators[m_frameIndex]->Reset() != S_OK) {
        return false;
    }
    if (m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr) != S_OK) {
        return false;
    }
    return true;
}

void DirectX12Renderer::beginRenderPass() {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_mainRenderTargetResource[m_backBufferIndex].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_commandList->ResourceBarrier(1, &barrier);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_commandList->ClearRenderTargetView(m_mainRenderTargetDescriptor[m_backBufferIndex], clearColor, 0, nullptr);
    m_commandList->OMSetRenderTargets(1, &m_mainRenderTargetDescriptor[m_backBufferIndex], FALSE, nullptr);
    m_commandList->SetDescriptorHeaps(1, m_srvDescriptorHeap.GetAddressOf());
}

void DirectX12Renderer::endRenderPass() {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_mainRenderTargetResource[m_backBufferIndex].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_commandList->ResourceBarrier(1, &barrier);

    if (m_commandList->Close() != S_OK) {
        return;
    }

    ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, cmdLists);

    m_fenceValues[m_frameIndex] = ++m_fenceLastSignaledValue;
    m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_frameIndex]);
}

void DirectX12Renderer::endFrame() {
    m_swapChain->Present(1, 0);
}

} // namespace aether

#endif // _WIN32 && AETHER_DIRECTX_12
