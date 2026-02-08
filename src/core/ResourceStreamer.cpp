#include "../../include/aether/ResourceStreamer.h"
#include "../../include/aether/WorkspaceManager.h"
#include <iostream>
// #include <algorithm>

namespace aether {

ResourceStreamer &ResourceStreamer::getInstance() {
  static ResourceStreamer instance;
  return instance;
}

bool ResourceStreamer::initialize() {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_initialized) {
    return true;
  }

  // Register all workspaces
  registerWorkspace(WorkspaceType::Edit);
  registerWorkspace(WorkspaceType::Animation);
  registerWorkspace(WorkspaceType::Photo);
  registerWorkspace(WorkspaceType::Color);
  registerWorkspace(WorkspaceType::Audio);
  registerWorkspace(WorkspaceType::Deliver);

  m_initialized = true;
  std::cout << "Resource Streamer initialized" << std::endl;
  return true;
}

void ResourceStreamer::shutdown() {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  // Stream out all workspaces
  for (auto &[workspace, info] : m_workspaceResources) {
    streamOutWorkspace(workspace);
  }

  m_workspaceResources.clear();
  m_initialized = false;
}

void ResourceStreamer::registerWorkspace(WorkspaceType workspace) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) == m_workspaceResources.end()) {
    m_workspaceResources[workspace] = ResourceInfo{};
  }
}

void ResourceStreamer::unregisterWorkspace(WorkspaceType workspace) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  streamOutWorkspace(workspace);
  m_workspaceResources.erase(workspace);
}

void ResourceStreamer::addResource(WorkspaceType workspace, uint64_t vramSize,
                                   uint64_t ramSize) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) != m_workspaceResources.end()) {
    auto &info = m_workspaceResources[workspace];
    info.vramSize += vramSize;
    info.ramSize += ramSize;
    info.isLoaded = true;
  }
}

void ResourceStreamer::removeResource(WorkspaceType workspace,
                                      uint64_t vramSize, uint64_t ramSize) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) != m_workspaceResources.end()) {
    auto &info = m_workspaceResources[workspace];
    if (info.vramSize >= vramSize) {
      info.vramSize -= vramSize;
    } else {
      info.vramSize = 0;
    }

    if (info.ramSize >= ramSize) {
      info.ramSize -= ramSize;
    } else {
      info.ramSize = 0;
    }

    if (info.vramSize == 0 && info.ramSize == 0) {
      info.isLoaded = false;
    }
  }
}

void ResourceStreamer::updateWorkspaceResources(WorkspaceType workspace) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  // Update active workspace
  auto &workspaceManager = WorkspaceManager::getInstance();
  m_activeWorkspace = workspaceManager.getCurrentWorkspace();

  // Auto-streaming: stream out inactive workspaces
  if (m_autoStreaming) {
    streamOutInactiveWorkspaces();
  }
}

void ResourceStreamer::streamOutWorkspace(WorkspaceType workspace) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) == m_workspaceResources.end()) {
    return;
  }

  auto &info = m_workspaceResources[workspace];

  if (!info.isLoaded) {
    return; // Already streamed out
  }

  // Cleanup workspace resources
  cleanupWorkspaceResources(workspace);

  // Reset resource info
  info.vramSize = 0;
  info.ramSize = 0;
  info.isLoaded = false;

  std::cout << "Streamed out workspace: " << static_cast<int>(workspace)
            << std::endl;
}

void ResourceStreamer::streamInWorkspace(WorkspaceType workspace) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) == m_workspaceResources.end()) {
    registerWorkspace(workspace);
  }

  auto &info = m_workspaceResources[workspace];

  if (info.isLoaded) {
    return; // Already streamed in
  }

  // Workspace will load its resources when activated
  // This is handled by WorkspaceManager
  info.isLoaded = true;

  std::cout << "Streamed in workspace: " << static_cast<int>(workspace)
            << std::endl;
}

void ResourceStreamer::streamOutInactiveWorkspaces() {
  for (auto &[workspace, info] : m_workspaceResources) {
    if (workspace != m_activeWorkspace && info.isLoaded) {
      streamOutWorkspace(workspace);
    }
  }
}

const ResourceInfo &
ResourceStreamer::getWorkspaceResources(WorkspaceType workspace) const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  static ResourceInfo emptyInfo;
  if (m_workspaceResources.find(workspace) != m_workspaceResources.end()) {
    return m_workspaceResources.at(workspace);
  }

  return emptyInfo;
}

uint64_t ResourceStreamer::getTotalVRAMUsage() const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  uint64_t total = 0;
  for (const auto &[workspace, info] : m_workspaceResources) {
    total += info.vramSize;
  }

  return total;
}

uint64_t ResourceStreamer::getTotalRAMUsage() const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  uint64_t total = 0;
  for (const auto &[workspace, info] : m_workspaceResources) {
    total += info.ramSize;
  }

  return total;
}

uint64_t
ResourceStreamer::getWorkspaceVRAMUsage(WorkspaceType workspace) const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_workspaceResources.find(workspace) != m_workspaceResources.end()) {
    return m_workspaceResources.at(workspace).vramSize;
  }

  return 0;
}

void ResourceStreamer::cleanupWorkspaceResources(WorkspaceType workspace) {
  // Placeholder for actual cleanup
  // In a full implementation, this would:
  // 1. Release Vulkan textures/buffers
  // 2. Clear GPU memory
  // 3. Unload workspace-specific data

  std::cout << "Cleaning up resources for workspace: "
            << static_cast<int>(workspace) << std::endl;
}

} // namespace aether
