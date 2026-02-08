#include "../../include/aether/Tool.h"
#include <iostream>
#include <map>

namespace aether {

// Tool implementation
Tool::Tool(ToolType type, const std::string &name, int shortcutKey)
    : m_type(type), m_name(name), m_shortcutKey(shortcutKey) {}

void Tool::activate() {
  m_isActive = true;
  std::cout << "Tool activated: " << m_name << std::endl;
}

void Tool::deactivate() {
  m_isActive = false;
  std::cout << "Tool deactivated: " << m_name << std::endl;
}

void Tool::update(float deltaTime) {
  (void)deltaTime;
  // Override in derived classes
}

void Tool::render() {
  // Override in derived classes
}

bool Tool::onMouseDown(int button, int x, int y) { return false; }

bool Tool::onMouseUp(int button, int x, int y) { return false; }

bool Tool::onMouseMove(int x, int y) { return false; }

bool Tool::onKeyPress(int key) { return false; }

// ToolRegistry implementation
ToolRegistry &ToolRegistry::getInstance() {
  static ToolRegistry instance;
  return instance;
}

bool ToolRegistry::registerTool(ToolType type, ToolFactory factory) {
  m_toolFactories[type] = factory;
  return true;
}

std::unique_ptr<Tool> ToolRegistry::createTool(ToolType type) {
  if (m_toolFactories.find(type) != m_toolFactories.end()) {
    return m_toolFactories[type]();
  }
  return nullptr;
}

Tool *ToolRegistry::getTool(ToolType type) {
  // Create tool instance if not exists
  if (m_toolInstances.find(type) == m_toolInstances.end()) {
    auto tool = createTool(type);
    if (tool) {
      m_toolInstances[type] = std::move(tool);
    }
  }

  if (m_toolInstances.find(type) != m_toolInstances.end()) {
    return m_toolInstances[type].get();
  }

  return nullptr;
}

const std::string &ToolRegistry::getToolName(ToolType type) const {
  static const std::map<ToolType, std::string> toolNames = {
      {ToolType::Selection, "Selection"},
      {ToolType::Blade, "Blade"},
      {ToolType::Ripple, "Ripple"},
      {ToolType::Slip, "Slip"},
      {ToolType::Text, "Text"},
      {ToolType::Transform, "Transform"},
      {ToolType::Pen, "Pen"},
      {ToolType::PuppetPin, "Puppet Pin"},
      {ToolType::MagicTracker, "Magic Tracker"},
      {ToolType::Healing, "Healing"},
      {ToolType::Clone, "Clone"},
      {ToolType::Brush, "Brush"},
      {ToolType::Gradient, "Gradient"},
      {ToolType::Qualifier, "Qualifier"},
      {ToolType::Windows, "Windows"},
      {ToolType::Range, "Range"},
      {ToolType::Envelope, "Envelope"},
      {ToolType::SpectralCleaner, "Spectral Cleaner"}};

  if (toolNames.find(type) != toolNames.end()) {
    return toolNames.at(type);
  }

  static const std::string unknown = "Unknown";
  return unknown;
}

int ToolRegistry::getToolShortcut(ToolType type) const {
  static const std::map<ToolType, int> shortcuts = {
      {ToolType::Selection, 'V'}, {ToolType::Blade, 'C'},
      {ToolType::Ripple, 'B'},    {ToolType::Slip, 'Y'},
      {ToolType::Text, 'T'},      {ToolType::Transform, 'V'},
      {ToolType::Pen, 'P'},       {ToolType::Healing, 'H'},
      {ToolType::Clone, 'S'},     {ToolType::Brush, 'B'},
      {ToolType::Gradient, 'G'},  {ToolType::Qualifier, 'L'},
      {ToolType::Windows, 'W'},   {ToolType::Range, 'A'},
      {ToolType::Envelope, 'P'}};

  if (shortcuts.find(type) != shortcuts.end()) {
    return shortcuts.at(type);
  }

  return 0;
}

std::vector<ToolType> ToolRegistry::getAvailableTools() const {
  std::vector<ToolType> tools;
  for (const auto &[type, factory] : m_toolFactories) {
    tools.push_back(type);
  }
  return tools;
}

// Initialize default tools
void ToolRegistry::initializeDefaultTools() {
  // Register all default tools
  registerTool(ToolType::Selection, []() {
    return std::make_unique<Tool>(ToolType::Selection, "Selection", 'V');
  });

  registerTool(ToolType::Blade, []() {
    return std::make_unique<Tool>(ToolType::Blade, "Blade", 'C');
  });

  registerTool(ToolType::Ripple, []() {
    return std::make_unique<Tool>(ToolType::Ripple, "Ripple", 'B');
  });

  registerTool(ToolType::Slip, []() {
    return std::make_unique<Tool>(ToolType::Slip, "Slip", 'Y');
  });

  registerTool(ToolType::Text, []() {
    return std::make_unique<Tool>(ToolType::Text, "Text", 'T');
  });

  registerTool(ToolType::Transform, []() {
    return std::make_unique<Tool>(ToolType::Transform, "Transform", 'V');
  });

  registerTool(ToolType::Pen, []() {
    return std::make_unique<Tool>(ToolType::Pen, "Pen", 'P');
  });

  registerTool(ToolType::PuppetPin, []() {
    return std::make_unique<Tool>(ToolType::PuppetPin, "Puppet Pin", 0);
  });

  registerTool(ToolType::MagicTracker, []() {
    return std::make_unique<Tool>(ToolType::MagicTracker, "Magic Tracker", 0);
  });

  registerTool(ToolType::Healing, []() {
    return std::make_unique<Tool>(ToolType::Healing, "Healing", 'H');
  });

  registerTool(ToolType::Clone, []() {
    return std::make_unique<Tool>(ToolType::Clone, "Clone", 'S');
  });

  registerTool(ToolType::Brush, []() {
    return std::make_unique<Tool>(ToolType::Brush, "Brush", 'B');
  });

  registerTool(ToolType::Gradient, []() {
    return std::make_unique<Tool>(ToolType::Gradient, "Gradient", 'G');
  });

  registerTool(ToolType::Qualifier, []() {
    return std::make_unique<Tool>(ToolType::Qualifier, "Qualifier", 'L');
  });

  registerTool(ToolType::Windows, []() {
    return std::make_unique<Tool>(ToolType::Windows, "Windows", 'W');
  });

  registerTool(ToolType::Range, []() {
    return std::make_unique<Tool>(ToolType::Range, "Range", 'A');
  });

  registerTool(ToolType::Envelope, []() {
    return std::make_unique<Tool>(ToolType::Envelope, "Envelope", 'P');
  });

  registerTool(ToolType::SpectralCleaner, []() {
    return std::make_unique<Tool>(ToolType::SpectralCleaner, "Spectral Cleaner",
                                  0);
  });
}

} // namespace aether
