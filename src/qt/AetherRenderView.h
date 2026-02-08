#pragma once

#include <QVulkanWindow>
#include <QVulkanWindowRenderer>

namespace aether {

class AetherVulkanRenderer;

class AetherRenderView : public QVulkanWindow {
    Q_OBJECT
public:
    explicit AetherRenderView(QWindow* parent = nullptr);
    ~AetherRenderView() override = default;

    QVulkanWindowRenderer* createRenderer() override;
};

// Minimal renderer: init/release stubs, startNextFrame just signals frame ready.
class AetherVulkanRenderer : public QVulkanWindowRenderer {
public:
    explicit AetherVulkanRenderer(QVulkanWindow* w);
    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

private:
    QVulkanWindow* m_window = nullptr;
};

} // namespace aether
