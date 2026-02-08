#include "../../include/aether/GlobalProjectContext.h"
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>


using json = nlohmann::json;

namespace aether {

GlobalProjectContext &GlobalProjectContext::getInstance() {
  static GlobalProjectContext instance;
  return instance;
}

bool GlobalProjectContext::createNewProject(const ProjectSettings &settings) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!settings.isValid()) {
    std::cerr << "Invalid project settings" << std::endl;
    return false;
  }

  // Close current project if any
  if (m_state != ProjectState::None) {
    closeProject();
  }

  m_settings = settings;
  m_state = ProjectState::New;
  m_projectPath.clear();
  m_createdTime = std::chrono::system_clock::now();
  m_modifiedTime = m_createdTime;
  m_lastSavedTime = {};

  std::cout << "Created new project: " << m_settings.projectName << std::endl;
  return true;
}

bool GlobalProjectContext::loadProject(const std::string &filePath) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!std::filesystem::exists(filePath)) {
    std::cerr << "Project file does not exist: " << filePath << std::endl;
    return false;
  }

  // Close current project if any
  if (m_state != ProjectState::None) {
    closeProject();
  }

  try {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open project file: " << filePath << std::endl;
      return false;
    }

    json j;
    file >> j;
    file.close();

    // Parse project data
    if (j.contains("projectName")) {
      m_settings.projectName = j["projectName"].get<std::string>();
    } else {
      m_settings.projectName = std::filesystem::path(filePath).stem().string();
    }

    if (j.contains("width")) {
      m_settings.width = j["width"].get<uint32_t>();
    }
    if (j.contains("height")) {
      m_settings.height = j["height"].get<uint32_t>();
    }
    if (j.contains("fps")) {
      m_settings.fps = j["fps"].get<uint32_t>();
    }
    if (j.contains("projectPath")) {
      m_settings.projectPath = j["projectPath"].get<std::string>();
    }

    // Parse timestamps
    if (j.contains("createdTime")) {
      auto timeStr = j["createdTime"].get<std::string>();
      m_createdTime = std::chrono::system_clock::now();
    }
    if (j.contains("modifiedTime")) {
      m_modifiedTime = std::chrono::system_clock::now();
    }
    if (j.contains("lastSavedTime")) {
      m_lastSavedTime = std::chrono::system_clock::now();
    }

    m_projectPath = filePath;
    m_state = ProjectState::Loaded;

    if (m_createdTime.time_since_epoch().count() == 0) {
      m_createdTime = std::chrono::system_clock::now();
    }
    if (m_modifiedTime.time_since_epoch().count() == 0) {
      m_modifiedTime = m_createdTime;
    }
    if (m_lastSavedTime.time_since_epoch().count() == 0) {
      m_lastSavedTime = m_modifiedTime;
    }

    std::cout << "Loaded project: " << m_projectPath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading project: " << e.what() << std::endl;
    return false;
  }
}

bool GlobalProjectContext::saveProject() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_state == ProjectState::None) {
    std::cerr << "No project to save" << std::endl;
    return false;
  }

  if (m_projectPath.empty()) {
    std::cerr << "Project path not set, use saveProjectAs()" << std::endl;
    return false;
  }

  return saveProjectAs(m_projectPath);
}

bool GlobalProjectContext::saveProjectAs(const std::string &filePath) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_state == ProjectState::None) {
    std::cerr << "No project to save" << std::endl;
    return false;
  }

  m_state = ProjectState::Saving;

  try {
    // Create directory if it doesn't exist
    std::filesystem::path path(filePath);
    std::filesystem::create_directories(path.parent_path());

    // Create JSON object
    json j;
    j["version"] = "1.0";
    j["projectName"] = m_settings.projectName;
    j["width"] = m_settings.width;
    j["height"] = m_settings.height;
    j["fps"] = m_settings.fps;
    j["projectPath"] = m_settings.projectPath;

    // Serialize timestamps
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S");

    if (m_createdTime.time_since_epoch().count() != 0) {
      auto createdT = std::chrono::system_clock::to_time_t(m_createdTime);
      std::stringstream createdSs;
      createdSs << std::put_time(std::localtime(&createdT),
                                 "%Y-%m-%d %H:%M:%S");
      j["createdTime"] = createdSs.str();
    } else {
      j["createdTime"] = ss.str();
      m_createdTime = now;
    }

    j["modifiedTime"] = ss.str();
    j["lastSavedTime"] = ss.str();

    // Write JSON to file
    std::ofstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open file for writing: " << filePath << std::endl;
      m_state = ProjectState::Error;
      return false;
    }

    file << j.dump(4); // Pretty print with 4 spaces indentation
    file.close();

    m_projectPath = filePath;
    m_state = ProjectState::Loaded;
    m_lastSavedTime = std::chrono::system_clock::now();
    m_modifiedTime = m_lastSavedTime;

    std::cout << "Project saved: " << m_projectPath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving project: " << e.what() << std::endl;
    m_state = ProjectState::Error;
    return false;
  }
}

void GlobalProjectContext::closeProject() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_state == ProjectState::None) {
    return;
  }

  // Reset all project data
  m_settings = ProjectSettings{};
  m_state = ProjectState::None;
  m_projectPath.clear();
  m_createdTime = {};
  m_modifiedTime = {};
  m_lastSavedTime = {};

  std::cout << "Project closed" << std::endl;
}

void GlobalProjectContext::markModified() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_state == ProjectState::Loaded || m_state == ProjectState::New) {
    m_state = ProjectState::Modified;
    m_modifiedTime = std::chrono::system_clock::now();
  }
}

} // namespace aether
