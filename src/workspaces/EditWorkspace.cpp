#include "EditWorkspace.h"
#include "../../include/aether/Tool.h"
#include "../../include/aether/WorkspaceManager.h"
#include <iostream>


namespace aether {

EditWorkspace::EditWorkspace() : Workspace(WorkspaceType::Edit) {
  m_name = "Edit";
}

void EditWorkspace::activate() {
  if (m_isActive) {
    return;
  }

  m_isActive = true;
  loadTools();

  std::cout << "Edit Workspace activated" << std::endl;
}

void EditWorkspace::deactivate() {
  if (!m_isActive) {
    return;
  }

  m_isActive = false;
  unloadTools();

  std::cout << "Edit Workspace deactivated" << std::endl;
}

void EditWorkspace::update(float deltaTime) {
  (void)deltaTime;
  if (!m_isActive) {
    return;
  }

  // Update edit workspace logic
  // Timeline, clips, effects, etc.
}

void EditWorkspace::render() {
  if (!m_isActive) {
    return;
  }

  // Render edit workspace UI
  // Timeline, preview, clip bins, etc.
}

void EditWorkspace::loadTools() {
  if (m_toolsLoaded) {
    return;
  }

  // Load edit-specific tools
  // Cut, trim, split, ripple, roll, etc.

  m_toolsLoaded = true;
  std::cout << "Edit workspace tools loaded" << std::endl;
}

void EditWorkspace::unloadTools() {
  if (!m_toolsLoaded) {
    return;
  }

  // Unload edit-specific tools

  m_toolsLoaded = false;
  std::cout << "Edit workspace tools unloaded" << std::endl;
}

} // namespace aether
