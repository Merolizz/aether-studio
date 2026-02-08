#include "../../include/aether/WorkspaceManager.h"
#include "../../include/aether/KeymapManager.h"
#include "../../include/aether/ResourceStreamer.h"
#include "../../include/aether/Tool.h"
#include "../workspaces/ColorWorkspace.h"
#include "../workspaces/EditWorkspace.h"
#include "../workspaces/PhotoWorkspace.h"
#include "imgui.h"
#include <algorithm>
#include <iostream>

namespace aether {

// Base Workspace implementation
Workspace::Workspace(WorkspaceType type) : m_type(type) {
  switch (type) {
  case WorkspaceType::Edit:
    m_name = "Edit";
    break;
  case WorkspaceType::Animation:
    m_name = "Animation";
    break;
  case WorkspaceType::Photo:
    m_name = "Photo";
    break;
  case WorkspaceType::Color:
    m_name = "Color";
    break;
  case WorkspaceType::Audio:
    m_name = "Audio";
    break;
  case WorkspaceType::Deliver:
    m_name = "Deliver";
    break;
  }
}

void Workspace::activate() {
  m_isActive = true;
  loadTools();

  // Stream in workspace resources
  auto &resourceStreamer = ResourceStreamer::getInstance();
  resourceStreamer.streamInWorkspace(m_type);

  std::cout << "Workspace activated: " << m_name << std::endl;
}

void Workspace::deactivate() {
  unloadTools();

  // Stream out workspace resources
  auto &resourceStreamer = ResourceStreamer::getInstance();
  resourceStreamer.streamOutWorkspace(m_type);

  m_isActive = false;
  std::cout << "Workspace deactivated: " << m_name << std::endl;
}

void Workspace::update(float deltaTime) {
  (void)deltaTime; // Override in derived classes
}

void Workspace::render() {
  // Default workspace content
  ImGui::Text("Welcome to %s workspace", m_name.c_str());
  ImGui::Spacing();
  ImGui::Text("This workspace is under development.");
  ImGui::Text("Workspace-specific features will be implemented here.");
}

void Workspace::loadTools() {
  // Load tools based on workspace type
  auto &toolRegistry = ToolRegistry::getInstance();

  switch (m_type) {
  case WorkspaceType::Edit:
    toolRegistry.getTool(ToolType::Selection);
    toolRegistry.getTool(ToolType::Blade);
    toolRegistry.getTool(ToolType::Ripple);
    toolRegistry.getTool(ToolType::Slip);
    toolRegistry.getTool(ToolType::Text);
    break;
  case WorkspaceType::Animation:
    toolRegistry.getTool(ToolType::Transform);
    toolRegistry.getTool(ToolType::Pen);
    toolRegistry.getTool(ToolType::PuppetPin);
    toolRegistry.getTool(ToolType::MagicTracker);
    break;
  case WorkspaceType::Photo:
    toolRegistry.getTool(ToolType::Healing);
    toolRegistry.getTool(ToolType::Clone);
    toolRegistry.getTool(ToolType::Brush);
    toolRegistry.getTool(ToolType::Gradient);
    break;
  case WorkspaceType::Color:
    toolRegistry.getTool(ToolType::Qualifier);
    toolRegistry.getTool(ToolType::Windows);
    break;
  case WorkspaceType::Audio:
    toolRegistry.getTool(ToolType::Range);
    toolRegistry.getTool(ToolType::Envelope);
    toolRegistry.getTool(ToolType::SpectralCleaner);
    break;
  default:
    break;
  }
}

void Workspace::unloadTools() {
  // Tools are managed by ToolRegistry, no need to unload
}

// WorkspaceManager implementation
WorkspaceManager &WorkspaceManager::getInstance() {
  static WorkspaceManager instance;
  return instance;
}

bool WorkspaceManager::initialize() {
  std::cout << "Debug: WorkspaceManager::initialize start" << std::endl;
  if (m_initialized) {
    return true;
  }

  std::cout << "Debug: Creating default workspaces" << std::endl;
  createDefaultWorkspaces();

  // Activate default workspace
  if (!m_workspaces.empty()) {
    std::cout << "Debug: Switching to default workspace" << std::endl;
    switchToWorkspace(WorkspaceType::Edit);
  } else {
    std::cerr << "Debug: No workspaces created!" << std::endl;
  }

  m_initialized = true;
  std::cout << "Debug: WorkspaceManager initialized" << std::endl;
  return true;
}

void WorkspaceManager::shutdown() {
  // Deactivate current workspace
  if (m_currentWorkspace != WorkspaceType::Edit || !m_workspaces.empty()) {
    auto *current = getCurrentWorkspaceInstance();
    if (current) {
      current->deactivate();
    }
  }

  m_workspaces.clear();
  m_workspaceFactories.clear();
  m_initialized = false;
}

void WorkspaceManager::createDefaultWorkspaces() {
  std::cout << "Debug: Creating EditWorkspace" << std::endl;
  // Create specialized workspace instances
  m_workspaces[WorkspaceType::Edit] = std::make_unique<EditWorkspace>();

  std::cout << "Debug: Creating PhotoWorkspace" << std::endl;
  m_workspaces[WorkspaceType::Photo] = std::make_unique<PhotoWorkspace>();

  std::cout << "Debug: Creating ColorWorkspace" << std::endl;
  m_workspaces[WorkspaceType::Color] = std::make_unique<ColorWorkspace>();

  std::cout << "Debug: Creating Base Workspaces" << std::endl;
  // Create base workspace instances for other types (Animation, Audio, Deliver)
  m_workspaces[WorkspaceType::Animation] =
      std::make_unique<Workspace>(WorkspaceType::Animation);
  m_workspaces[WorkspaceType::Audio] =
      std::make_unique<Workspace>(WorkspaceType::Audio);
  m_workspaces[WorkspaceType::Deliver] =
      std::make_unique<Workspace>(WorkspaceType::Deliver);

  std::cout << "Default workspaces created (Edit, Photo, Color modules)"
            << std::endl;
}

bool WorkspaceManager::registerWorkspace(WorkspaceType type,
                                         WorkspaceFactory factory) {
  m_workspaceFactories[type] = factory;

  // Create instance if not exists
  if (m_workspaces.find(type) == m_workspaces.end()) {
    if (factory) {
      m_workspaces[type] = factory();
    } else {
      m_workspaces[type] = std::make_unique<Workspace>(type);
    }
  }

  return true;
}

bool WorkspaceManager::switchToWorkspace(WorkspaceType type) {
  std::cout << "Debug: Switching to workspace type " << static_cast<int>(type)
            << std::endl;
  if (m_workspaces.find(type) == m_workspaces.end()) {
    std::cerr << "Workspace not found: " << static_cast<int>(type) << std::endl;
    return false;
  }

  // Deactivate current workspace
  auto *current = getCurrentWorkspaceInstance();
  if (current && current->isActive()) {
    std::cout << "Debug: Deactivating current workspace" << std::endl;
    current->deactivate();
  }

  // Activate new workspace
  m_currentWorkspace = type;
  auto *newWorkspace = getCurrentWorkspaceInstance();
  if (newWorkspace) {
    std::cout << "Debug: Activating new workspace" << std::endl;
    newWorkspace->activate();
    std::cout << "Debug: New workspace activated" << std::endl;

    // Update keymap context
    auto &keymapManager = KeymapManager::getInstance();
    (void)keymapManager;
    // Keymap context will be updated based on workspace type
  }

  std::cout << "Switched to workspace: " << getWorkspaceName(type) << std::endl;
  return true;
}

Workspace *WorkspaceManager::getCurrentWorkspaceInstance() const {
  if (m_workspaces.find(m_currentWorkspace) != m_workspaces.end()) {
    return m_workspaces.at(m_currentWorkspace).get();
  }
  return nullptr;
}

Workspace *WorkspaceManager::getWorkspace(WorkspaceType type) const {
  if (m_workspaces.find(type) != m_workspaces.end()) {
    return m_workspaces.at(type).get();
  }
  return nullptr;
}

std::vector<WorkspaceType> WorkspaceManager::getAvailableWorkspaces() const {
  std::vector<WorkspaceType> workspaces;
  for (const auto &[type, workspace] : m_workspaces) {
    workspaces.push_back(type);
  }
  return workspaces;
}

const std::string &
WorkspaceManager::getWorkspaceName(WorkspaceType type) const {
  auto *workspace = getWorkspace(type);
  if (workspace) {
    return workspace->getName();
  }

  static const std::string unknown = "Unknown";
  return unknown;
}

std::string WorkspaceManager::getWorkspaceTypeName(WorkspaceType type) const {
  switch (type) {
  case WorkspaceType::Edit:
    return "Edit";
  case WorkspaceType::Animation:
    return "Animation";
  case WorkspaceType::Photo:
    return "Photo";
  case WorkspaceType::Color:
    return "Color";
  case WorkspaceType::Audio:
    return "Audio";
  case WorkspaceType::Deliver:
    return "Deliver";
  default:
    return "Unknown";
  }
}

void WorkspaceManager::update(float deltaTime) {
  auto *current = getCurrentWorkspaceInstance();
  if (current && current->isActive()) {
    current->update(deltaTime);
  }
}

void WorkspaceManager::render() {
  auto *current = getCurrentWorkspaceInstance();
  if (current && current->isActive()) {
    current->render();
  }
}

} // namespace aether
