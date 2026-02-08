#include "PhotoWorkspace.h"
#include "../../include/aether/Tool.h"
#include "../../include/aether/WorkspaceManager.h"
#include <iostream>


namespace aether {

PhotoWorkspace::PhotoWorkspace() : Workspace(WorkspaceType::Photo) {
  m_name = "Photo";
}

void PhotoWorkspace::activate() {
  if (m_isActive) {
    return;
  }

  m_isActive = true;
  loadTools();

  std::cout << "Photo Workspace activated" << std::endl;
}

void PhotoWorkspace::deactivate() {
  if (!m_isActive) {
    return;
  }

  m_isActive = false;
  unloadTools();

  std::cout << "Photo Workspace deactivated" << std::endl;
}

void PhotoWorkspace::update(float deltaTime) {
  (void)deltaTime; // Reserved for future animation/transition updates
  if (!m_isActive) {
    return;
  }

  // Update photo workspace logic
  // Layers, filters, adjustments, etc.
}

void PhotoWorkspace::render() {
  if (!m_isActive) {
    return;
  }

  // Render photo workspace UI
  // Canvas, layers panel, tools panel, etc.
}

void PhotoWorkspace::loadTools() {
  if (m_toolsLoaded) {
    return;
  }

  // Load photo-specific tools
  // Brush, clone, heal, crop, transform, etc.

  m_toolsLoaded = true;
  std::cout << "Photo workspace tools loaded" << std::endl;
}

void PhotoWorkspace::unloadTools() {
  if (!m_toolsLoaded) {
    return;
  }

  // Unload photo-specific tools

  m_toolsLoaded = false;
  std::cout << "Photo workspace tools unloaded" << std::endl;
}

} // namespace aether
