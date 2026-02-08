#include "Toolbar.h"
#include "../../include/aether/KeymapManager.h"
#include "../../include/aether/Tool.h"
#include "imgui.h"
#include <algorithm>


namespace aether {

Toolbar::Toolbar() { loadWorkspaceTools(m_currentWorkspace); }

void Toolbar::render() {
  ImGui::Begin("Toolbar", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

  auto &toolRegistry = ToolRegistry::getInstance();
  (void)toolRegistry; // Reserved for future tool validation

  for (auto toolType : m_currentTools) {
    renderToolButton(toolType);
  }

  ImGui::End();
}

void Toolbar::setWorkspace(WorkspaceType workspace) {
  if (m_currentWorkspace != workspace) {
    m_currentWorkspace = workspace;
    loadWorkspaceTools(workspace);
  }
}

void Toolbar::activateTool(ToolType toolType) {
  // Deactivate previous tool
  auto &toolRegistry = ToolRegistry::getInstance();
  Tool *prevTool = toolRegistry.getTool(m_activeTool);
  if (prevTool && prevTool->isActive()) {
    prevTool->deactivate();
  }

  // Activate new tool
  m_activeTool = toolType;
  Tool *newTool = toolRegistry.getTool(toolType);
  if (newTool) {
    newTool->activate();
  }
}

void Toolbar::renderToolButton(ToolType toolType) {
  auto &toolRegistry = ToolRegistry::getInstance();
  const std::string &name = toolRegistry.getToolName(toolType);
  int shortcut = toolRegistry.getToolShortcut(toolType);

  bool isActive = (toolType == m_activeTool);

  if (isActive) {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4(0.1f, 0.5f, 0.9f, 1.0f));
  }

  std::string label = name;
  if (shortcut > 0) {
    label += " (" + std::string(1, static_cast<char>(shortcut)) + ")";
  }

  if (ImGui::Button(label.c_str(), ImVec2(100, 30))) {
    activateTool(toolType);
  }

  if (isActive) {
    ImGui::PopStyleColor(3);
  }

  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("%s", name.c_str());
  }
}

void Toolbar::loadWorkspaceTools(WorkspaceType workspace) {
  m_currentTools.clear();

  switch (workspace) {
  case WorkspaceType::Edit:
    m_currentTools = {ToolType::Selection, ToolType::Blade, ToolType::Ripple,
                      ToolType::Slip, ToolType::Text};
    break;
  case WorkspaceType::Animation:
    m_currentTools = {ToolType::Transform, ToolType::Pen, ToolType::PuppetPin,
                      ToolType::MagicTracker};
    break;
  case WorkspaceType::Photo:
    m_currentTools = {ToolType::Healing, ToolType::Clone, ToolType::Brush,
                      ToolType::Gradient};
    break;
  case WorkspaceType::Color:
    m_currentTools = {ToolType::Qualifier, ToolType::Windows};
    break;
  case WorkspaceType::Audio:
    m_currentTools = {ToolType::Range, ToolType::Envelope,
                      ToolType::SpectralCleaner};
    break;
  case WorkspaceType::Deliver:
    // No tools for Deliver workspace
    break;
  }

  // Set default active tool
  if (!m_currentTools.empty()) {
    m_activeTool = m_currentTools[0];
  }
}

std::vector<ToolType>
Toolbar::getToolsForWorkspace(WorkspaceType workspace) const {
  std::vector<ToolType> tools;

  switch (workspace) {
  case WorkspaceType::Edit:
    tools = {ToolType::Selection, ToolType::Blade, ToolType::Ripple,
             ToolType::Slip, ToolType::Text};
    break;
  case WorkspaceType::Animation:
    tools = {ToolType::Transform, ToolType::Pen, ToolType::PuppetPin,
             ToolType::MagicTracker};
    break;
  case WorkspaceType::Photo:
    tools = {ToolType::Healing, ToolType::Clone, ToolType::Brush,
             ToolType::Gradient};
    break;
  case WorkspaceType::Color:
    tools = {ToolType::Qualifier, ToolType::Windows};
    break;
  case WorkspaceType::Audio:
    tools = {ToolType::Range, ToolType::Envelope, ToolType::SpectralCleaner};
    break;
  default:
    break;
  }

  return tools;
}

} // namespace aether
