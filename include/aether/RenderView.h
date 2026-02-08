#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace aether {
class TilingRenderer;

enum class ShaderType {
    Vertex,
    Fragment,
    Compute,
    Geometry
};

struct Shader {
    VkShaderModule module = VK_NULL_HANDLE;
    ShaderType type;
    std::string source;
    std::string entryPoint = "main";
};

struct RenderViewSettings {
    uint32_t width = 1920;
    uint32_t height = 1080;
    float zoom = 1.0f;
    float panX = 0.0f;
    float panY = 0.0f;
    bool fitToWindow = true;
};

class RenderView {
public:
    RenderView();
    ~RenderView();

    RenderView(const RenderView&) = delete;
    RenderView& operator=(const RenderView&) = delete;

    // Initialization
    bool initialize(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool);
    void shutdown();

    // Shader management
    bool loadShader(const std::string& name, ShaderType type, const std::string& source);
    bool loadShaderFromFile(const std::string& name, ShaderType type, const std::string& filePath);
    void unloadShader(const std::string& name);
    Shader* getShader(const std::string& name);
    
    // Render pipeline
    bool createPipeline(const std::string& vertexShader, const std::string& fragmentShader);
    void destroyPipeline();
    VkPipeline getPipeline() const { return m_graphicsPipeline; }
    VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
    
    // Rendering
    void beginRender(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkExtent2D extent);
    void render(VkCommandBuffer commandBuffer);
    void endRender(VkCommandBuffer commandBuffer);
    
    // View settings
    void setSettings(const RenderViewSettings& settings) { m_settings = settings; }
    const RenderViewSettings& getSettings() const { return m_settings; }
    void setZoom(float zoom) { m_settings.zoom = zoom; }
    void setPan(float x, float y) { m_settings.panX = x; m_settings.panY = y; }
    
    // Effect management
    bool addEffect(const std::string& name, const std::string& shaderName);
    void removeEffect(const std::string& name);
    void clearEffects();
    
    // Tiling render support
    void enableTiling(bool enable) { m_useTiling = enable; }
    bool isTilingEnabled() const { return m_useTiling; }
    void setTileSize(uint32_t tileSize);

private:
    bool compileShader(const std::string& source, ShaderType type, VkShaderModule& module);
    bool createDescriptorSetLayout();
    bool createUniformBuffers();
    void updateUniformBuffer();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    
    std::map<std::string, Shader> m_shaders;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    
    RenderViewSettings m_settings;
    std::vector<std::string> m_activeEffects;
    
    // Tiling render
    bool m_useTiling = false;
    uint32_t m_tileSize = 512;
    std::unique_ptr<TilingRenderer> m_tilingRenderer;
    
    bool m_initialized = false;
};

} // namespace aether
