#include "AetherRenderView.h"
#include <QVulkanDeviceFunctions>
#include <QDebug>

namespace aether {

AetherRenderView::AetherRenderView(QWindow* parent)
    : QVulkanWindow(parent) {
}

QVulkanWindowRenderer* AetherRenderView::createRenderer() {
    return new AetherVulkanRenderer(this);
}

// --- AetherVulkanRenderer ---

AetherVulkanRenderer::AetherVulkanRenderer(QVulkanWindow* w)
    : m_window(w) {
}

void AetherVulkanRenderer::initResources() {
    if (!m_window || !m_window->vulkanInstance()) {
        qWarning("AetherVulkanRenderer::initResources: no window or Vulkan instance");
        return;
    }
}

void AetherVulkanRenderer::initSwapChainResources() {
}

void AetherVulkanRenderer::releaseSwapChainResources() {
}

void AetherVulkanRenderer::releaseResources() {
}

void AetherVulkanRenderer::startNextFrame() {
    if (!m_window) {
        return;
    }
    VkCommandBuffer cmdBuf = m_window->currentCommandBuffer();
    VkRenderPass pass = m_window->defaultRenderPass();
    VkFramebuffer fb = m_window->currentFramebuffer();

    if (cmdBuf == VK_NULL_HANDLE || pass == VK_NULL_HANDLE || fb == VK_NULL_HANDLE) {
        m_window->frameReady();
        return;
    }

    QVulkanInstance* inst = m_window->vulkanInstance();
    if (!inst) {
        m_window->frameReady();
        return;
    }
    QVulkanDeviceFunctions* funcs = inst->deviceFunctions(m_window->device());
    if (!funcs) {
        m_window->frameReady();
        return;
    }

    VkClearValue clearColor = {};
    clearColor.color.float32[0] = 0.12f;
    clearColor.color.float32[1] = 0.12f;
    clearColor.color.float32[2] = 0.14f;
    clearColor.color.float32[3] = 1.0f;
    VkClearValue clearDepth = {};
    clearDepth.depthStencil.depth = 1.0f;
    clearDepth.depthStencil.stencil = 0;
    VkClearValue clearValues[] = { clearColor, clearDepth };

    VkRenderPassBeginInfo rpBegin = {};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = pass;
    rpBegin.framebuffer = fb;
    rpBegin.renderArea.offset = { 0, 0 };
    rpBegin.renderArea.extent.width = static_cast<uint32_t>(m_window->swapChainImageSize().width());
    rpBegin.renderArea.extent.height = static_cast<uint32_t>(m_window->swapChainImageSize().height());
    rpBegin.clearValueCount = 2;
    rpBegin.pClearValues = clearValues;

    funcs->vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    funcs->vkCmdEndRenderPass(cmdBuf);

    m_window->frameReady();
}

} // namespace aether
