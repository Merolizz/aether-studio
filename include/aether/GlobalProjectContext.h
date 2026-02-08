#pragma once

#include "ProjectSettings.h"
#include <string>
#include <memory>
#include <mutex>
#include <chrono>

namespace aether {

enum class ProjectState {
    None,           // No project loaded
    New,            // New project created but not saved
    Loaded,         // Project loaded from file
    Modified,       // Project has unsaved changes
    Saving,         // Project is being saved
    Error           // Error state
};

class GlobalProjectContext {
public:
    // Singleton access
    static GlobalProjectContext& getInstance();
    
    // Delete copy constructor and assignment operator
    GlobalProjectContext(const GlobalProjectContext&) = delete;
    GlobalProjectContext& operator=(const GlobalProjectContext&) = delete;
    
    // Project management
    bool createNewProject(const ProjectSettings& settings);
    bool loadProject(const std::string& filePath);
    bool saveProject();
    bool saveProjectAs(const std::string& filePath);
    void closeProject();
    
    // Getters
    ProjectState getState() const { return m_state; }
    const ProjectSettings& getSettings() const { return m_settings; }
    const std::string& getProjectPath() const { return m_projectPath; }
    const std::string& getProjectName() const { return m_settings.projectName; }
    bool isProjectOpen() const { return m_state != ProjectState::None; }
    bool hasUnsavedChanges() const { 
        return m_state == ProjectState::Modified || m_state == ProjectState::New; 
    }
    
    // Resolution and FPS accessors (explicit methods as per requirements)
    uint32_t getResolutionWidth() const { return m_settings.width; }
    uint32_t getResolutionHeight() const { return m_settings.height; }
    uint32_t getFPS() const { return m_settings.fps; }
    
    // Setters
    void markModified();
    void setProjectPath(const std::string& path) { m_projectPath = path; }
    
    // Project metadata
    std::chrono::system_clock::time_point getCreatedTime() const { return m_createdTime; }
    std::chrono::system_clock::time_point getModifiedTime() const { return m_modifiedTime; }
    std::chrono::system_clock::time_point getLastSavedTime() const { return m_lastSavedTime; }

private:
    GlobalProjectContext() = default;
    ~GlobalProjectContext() = default;

    ProjectSettings m_settings;
    ProjectState m_state = ProjectState::None;
    std::string m_projectPath;
    
    std::chrono::system_clock::time_point m_createdTime;
    std::chrono::system_clock::time_point m_modifiedTime;
    std::chrono::system_clock::time_point m_lastSavedTime;
    
    mutable std::mutex m_mutex; // Thread safety
};

// Backward compatibility alias
using ProjectContext = GlobalProjectContext;

} // namespace aether
