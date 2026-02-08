#pragma once

#include "KeymapManager.h"
#include <string>
#include <functional>
#include <memory>
#include <map>

namespace aether {

enum class WorkspaceType {
    Edit,       // NLE - Kurgu
    Animation,  // VFX & Motion Graphics
    Photo,      // Photo/RAW - Photoshop/Lightroom
    Color,      // Color Grading
    Audio,      // Sound Editing
    Deliver     // Export/Render
};

class Workspace {
public:
    Workspace(WorkspaceType type);
    virtual ~Workspace() = default;

    Workspace(const Workspace&) = delete;
    Workspace& operator=(const Workspace&) = delete;

    // Lifecycle
    virtual void activate();
    virtual void deactivate();
    virtual void update(float deltaTime);
    virtual void render();
    
    // Getters
    WorkspaceType getType() const { return m_type; }
    const std::string& getName() const { return m_name; }
    bool isActive() const { return m_isActive; }
    
    // Tool management
    virtual void loadTools();
    virtual void unloadTools();
    
protected:
    WorkspaceType m_type;
    std::string m_name;
    bool m_isActive = false;
};

class WorkspaceManager {
public:
    using WorkspaceFactory = std::function<std::unique_ptr<Workspace>()>;
    
    // Singleton access
    static WorkspaceManager& getInstance();
    
    // Delete copy constructor and assignment operator
    WorkspaceManager(const WorkspaceManager&) = delete;
    WorkspaceManager& operator=(const WorkspaceManager&) = delete;

    // Initialization
    bool initialize();
    void shutdown();

    // Workspace management
    bool registerWorkspace(WorkspaceType type, WorkspaceFactory factory);
    bool switchToWorkspace(WorkspaceType type);
    WorkspaceType getCurrentWorkspace() const { return m_currentWorkspace; }
    Workspace* getCurrentWorkspaceInstance() const;
    Workspace* getWorkspace(WorkspaceType type) const;
    
    // Workspace list
    std::vector<WorkspaceType> getAvailableWorkspaces() const;
    const std::string& getWorkspaceName(WorkspaceType type) const;
    
    // Update and render
    void update(float deltaTime);
    void render();

private:
    WorkspaceManager() = default;
    ~WorkspaceManager() = default;

    void createDefaultWorkspaces();
    std::string getWorkspaceTypeName(WorkspaceType type) const;

    std::map<WorkspaceType, std::unique_ptr<Workspace>> m_workspaces;
    std::map<WorkspaceType, WorkspaceFactory> m_workspaceFactories;
    WorkspaceType m_currentWorkspace = WorkspaceType::Edit;
    bool m_initialized = false;
};

} // namespace aether
