#pragma once

#if defined(_WIN32) && defined(AETHER_DIRECTX_11)

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

namespace aether {
class Window;
}

namespace aether {

class DirectX11Renderer {
public:
    DirectX11Renderer();
    ~DirectX11Renderer();

    DirectX11Renderer(const DirectX11Renderer&) = delete;
    DirectX11Renderer& operator=(const DirectX11Renderer&) = delete;

    bool initialize(Window* window);
    void shutdown();

    bool beginFrame();
    void beginRenderPass();
    void endRenderPass();
    void endFrame();

    ID3D11Device* getDevice() const { return m_device.Get(); }
    ID3D11DeviceContext* getDeviceContext() const { return m_deviceContext.Get(); }
    ID3D11RenderTargetView* getRenderTargetView() const { return m_renderTargetView.Get(); }

private:
    bool createDeviceAndSwapChain(Window* window);
    bool createRenderTarget();
    void cleanupRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    UINT m_frameIndex = 0;
    Window* m_window = nullptr;
};

} // namespace aether

#endif // _WIN32 && AETHER_DIRECTX_11
