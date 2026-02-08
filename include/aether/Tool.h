#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>

namespace aether {

enum class ToolType {
    Selection,
    Blade,
    Ripple,
    Slip,
    Text,
    Transform,
    Pen,
    PuppetPin,
    MagicTracker,
    Healing,
    Clone,
    Brush,
    Gradient,
    Qualifier,
    Windows,
    Range,
    Envelope,
    SpectralCleaner
};

class Tool {
public:
    Tool(ToolType type, const std::string& name, int shortcutKey);
    virtual ~Tool() = default;

    Tool(const Tool&) = delete;
    Tool& operator=(const Tool&) = delete;

    // Tool lifecycle
    virtual void activate();
    virtual void deactivate();
    virtual void update(float deltaTime);
    virtual void render();
    
    // Input handling
    virtual bool onMouseDown(int button, int x, int y);
    virtual bool onMouseUp(int button, int x, int y);
    virtual bool onMouseMove(int x, int y);
    virtual bool onKeyPress(int key);
    
    // Getters
    ToolType getType() const { return m_type; }
    const std::string& getName() const { return m_name; }
    int getShortcutKey() const { return m_shortcutKey; }
    bool isActive() const { return m_isActive; }
    const std::string& getDescription() const { return m_description; }
    
    void setDescription(const std::string& desc) { m_description = desc; }

protected:
    ToolType m_type;
    std::string m_name;
    int m_shortcutKey;
    bool m_isActive = false;
    std::string m_description;
};

class ToolRegistry {
public:
    using ToolFactory = std::function<std::unique_ptr<Tool>()>;
    
    // Singleton access
    static ToolRegistry& getInstance();
    
    // Delete copy constructor and assignment operator
    ToolRegistry(const ToolRegistry&) = delete;
    ToolRegistry& operator=(const ToolRegistry&) = delete;

    // Tool registration
    bool registerTool(ToolType type, ToolFactory factory);
    std::unique_ptr<Tool> createTool(ToolType type);
    
    // Tool lookup
    Tool* getTool(ToolType type);
    const std::string& getToolName(ToolType type) const;
    int getToolShortcut(ToolType type) const;
    
    // Tool list
    std::vector<ToolType> getAvailableTools() const;
    
    // Initialization
    void initializeDefaultTools();

private:
    ToolRegistry() = default;
    ~ToolRegistry() = default;

    std::map<ToolType, ToolFactory> m_toolFactories;
    std::map<ToolType, std::unique_ptr<Tool>> m_toolInstances;
};

} // namespace aether
