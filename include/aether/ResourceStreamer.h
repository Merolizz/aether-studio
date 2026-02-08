#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <mutex>

// Need full definition for member variables/defaults.
#include "WorkspaceManager.h"

namespace aether {

struct ResourceInfo {
    uint64_t vramSize = 0;      // VRAM usage in bytes
    uint64_t ramSize = 0;       // RAM usage in bytes
    uint32_t textureCount = 0;
    uint32_t bufferCount = 0;
    bool isLoaded = false;
};

class ResourceStreamer {
public:
    // Singleton access
    static ResourceStreamer& getInstance();
    
    // Delete copy constructor and assignment operator
    ResourceStreamer(const ResourceStreamer&) = delete;
    ResourceStreamer& operator=(const ResourceStreamer&) = delete;

    // Initialization
    bool initialize();
    void shutdown();

    // Workspace resource management
    void registerWorkspace(WorkspaceType workspace);
    void unregisterWorkspace(WorkspaceType workspace);
    
    // Resource tracking
    void addResource(WorkspaceType workspace, uint64_t vramSize, uint64_t ramSize = 0);
    void removeResource(WorkspaceType workspace, uint64_t vramSize, uint64_t ramSize = 0);
    void updateWorkspaceResources(WorkspaceType workspace);
    
    // Streaming operations
    void streamOutWorkspace(WorkspaceType workspace);
    void streamInWorkspace(WorkspaceType workspace);
    void streamOutInactiveWorkspaces();
    
    // Queries
    const ResourceInfo& getWorkspaceResources(WorkspaceType workspace) const;
    uint64_t getTotalVRAMUsage() const;
    uint64_t getTotalRAMUsage() const;
    uint64_t getWorkspaceVRAMUsage(WorkspaceType workspace) const;
    
    // Settings
    void setAutoStreaming(bool enable) { m_autoStreaming = enable; }
    bool isAutoStreaming() const { return m_autoStreaming; }

private:
    ResourceStreamer() = default;
    ~ResourceStreamer() = default;

    void cleanupWorkspaceResources(WorkspaceType workspace);

    std::map<WorkspaceType, ResourceInfo> m_workspaceResources;
    WorkspaceType m_activeWorkspace = WorkspaceType::Edit;
    bool m_autoStreaming = true;
    bool m_initialized = false;
    
    mutable std::recursive_mutex m_mutex;
};

} // namespace aether
