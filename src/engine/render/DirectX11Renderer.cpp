#if defined(_WIN32) && defined(AETHER_DIRECTX_11)

#include "DirectX11Renderer.h"
#include "../../core/Window.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace aether {

DirectX11Renderer::DirectX11Renderer() {}

DirectX11Renderer::~DirectX11Renderer() {
    shutdown();
}

bool DirectX11Renderer::initialize(Window* window) {
    m_window = window;
    return createDeviceAndSwapChain(window);
}

void DirectX11Renderer::shutdown() {
    cleanupRenderTarget();
    m_swapChain.Reset();
    m_deviceContext.Reset();
    m_device.Reset();
    m_window = nullptr;
}

bool DirectX11Renderer::createDeviceAndSwapChain(Window* window) {
    HWND hwnd = glfwGetWin32Window(window->getHandle());
    if (!hwnd) {
        std::cerr << "Failed to get HWND from GLFW window" << std::endl;
        return false;
    }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    IDXGISwapChain* pSwapChain = nullptr;
    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
        &pSwapChain, &pDevice, &featureLevel, &pContext);

    if (res == DXGI_ERROR_UNSUPPORTED) {
        res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
            &pSwapChain, &pDevice, &featureLevel, &pContext);
    }
    if (res != S_OK) {
        std::cerr << "D3D11CreateDeviceAndSwapChain failed" << std::endl;
        return false;
    }

    m_swapChain.Attach(pSwapChain);
    m_device.Attach(pDevice);
    m_deviceContext.Attach(pContext);

    return createRenderTarget();
}

bool DirectX11Renderer::createRenderTarget() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (hr != S_OK) {
        std::cerr << "GetBuffer failed" << std::endl;
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (hr != S_OK) {
        std::cerr << "CreateRenderTargetView failed" << std::endl;
        return false;
    }
    return true;
}

void DirectX11Renderer::cleanupRenderTarget() {
    m_renderTargetView.Reset();
}

bool DirectX11Renderer::beginFrame() {
    return true;
}

void DirectX11Renderer::beginRenderPass() {
    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}

void DirectX11Renderer::endRenderPass() {
    // Nothing to do for DX11
}

void DirectX11Renderer::endFrame() {
    m_swapChain->Present(1, 0);
}

} // namespace aether

#endif // _WIN32 && AETHER_DIRECTX_11
