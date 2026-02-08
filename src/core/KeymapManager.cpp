#include "../../include/aether/KeymapManager.h"
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>


using json = nlohmann::json;

namespace aether {

KeymapManager &KeymapManager::getInstance() {
  static KeymapManager instance;
  return instance;
}

bool KeymapManager::initialize() {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_initialized) {
    return true;
  }

  // Create default keymap
  createDefaultKeymap();

  /*
  // Try to load saved keymap
  std::string keymapPath = getKeymapFilePath();
  if (std::filesystem::exists(keymapPath)) {
      if (!loadKeymap(keymapPath)) {
          std::cerr << "Failed to load keymap, using defaults" << std::endl;
      }
  }
  */

  m_initialized = true;
  std::cout << "Debug: KeymapManager fully initialized (Defaults)" << std::endl;
  return true;
}

void KeymapManager::shutdown() {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  // Save keymap on shutdown
  std::string keymapPath = getKeymapFilePath();
  saveKeymap(keymapPath);

  m_keyBindings.clear();
  m_actionBindings.clear();
  m_actionCallbacks.clear();
  m_actionDescriptions.clear();
  m_initialized = false;
}

void KeymapManager::createDefaultKeymap() {
  // Edit workspace shortcuts
  bindKey("edit.select", 'V', KeyModifier::None, WorkspaceContext::Edit);
  bindKey("edit.blade", 'C', KeyModifier::None, WorkspaceContext::Edit);
  bindKey("edit.ripple", 'B', KeyModifier::None, WorkspaceContext::Edit);
  bindKey("edit.slip", 'Y', KeyModifier::None, WorkspaceContext::Edit);
  bindKey("edit.text", 'T', KeyModifier::None, WorkspaceContext::Edit);

  // Animation workspace shortcuts
  bindKey("animation.transform", 'V', KeyModifier::None,
          WorkspaceContext::Animation);
  bindKey("animation.pen", 'P', KeyModifier::None, WorkspaceContext::Animation);

  // Photo workspace shortcuts
  bindKey("photo.healing", 'H', KeyModifier::None, WorkspaceContext::Photo);
  bindKey("photo.clone", 'S', KeyModifier::None, WorkspaceContext::Photo);
  bindKey("photo.brush", 'B', KeyModifier::None, WorkspaceContext::Photo);
  bindKey("photo.gradient", 'G', KeyModifier::None, WorkspaceContext::Photo);

  // Color workspace shortcuts
  bindKey("color.qualifier", 'L', KeyModifier::None, WorkspaceContext::Color);
  bindKey("color.windows", 'W', KeyModifier::None, WorkspaceContext::Color);

  // Audio workspace shortcuts
  bindKey("audio.range", 'A', KeyModifier::None, WorkspaceContext::Audio);
  bindKey("audio.envelope", 'P', KeyModifier::None, WorkspaceContext::Audio);

  // Global shortcuts
  bindKey("file.new", 'N', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("file.open", 'O', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("file.save", 'S', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("file.saveAs", 'S', KeyModifier::Ctrl | KeyModifier::Shift,
          WorkspaceContext::Global);
  bindKey("edit.undo", 'Z', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("edit.redo", 'Z', KeyModifier::Ctrl | KeyModifier::Shift,
          WorkspaceContext::Global);
  bindKey("edit.cut", 'X', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("edit.copy", 'C', KeyModifier::Ctrl, WorkspaceContext::Global);
  bindKey("edit.paste", 'V', KeyModifier::Ctrl, WorkspaceContext::Global);
}

std::string KeymapManager::getKeymapFilePath() const {
  // Get user config directory
  std::string configDir;

#ifdef _WIN32
  const char *appData = std::getenv("APPDATA");
  if (appData) {
    configDir = std::string(appData) + "\\AetherStudio";
  } else {
    configDir = ".\\config";
  }
#else
  const char *home = std::getenv("HOME");
  if (home) {
    configDir = std::string(home) + "/.config/AetherStudio";
  } else {
    configDir = "./config";
  }
#endif

  std::filesystem::create_directories(configDir);
  return configDir + "/keymap.json";
}

bool KeymapManager::loadKeymap(const std::string &filePath) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  try {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open keymap file: " << filePath << std::endl;
      return false;
    }

    json j;
    file >> j;
    file.close();

    // Clear existing bindings
    m_keyBindings.clear();
    m_actionBindings.clear();

    // Load bindings
    if (j.contains("bindings") && j["bindings"].is_array()) {
      for (const auto &binding : j["bindings"]) {
        std::string action = binding["action"].get<std::string>();
        int key = binding["key"].get<int>();
        int mods = binding["modifiers"].get<int>();
        int ctx = binding["context"].get<int>();

        KeyModifier modifiers = static_cast<KeyModifier>(mods);
        WorkspaceContext context = static_cast<WorkspaceContext>(ctx);

        bindKey(action, key, modifiers, context);

        if (binding.contains("description")) {
          m_actionDescriptions[action] =
              binding["description"].get<std::string>();
        }
      }
    }

    std::cout << "Keymap loaded from: " << filePath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error loading keymap: " << e.what() << std::endl;
    return false;
  }
}

bool KeymapManager::saveKeymap(const std::string &filePath) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  try {
    std::filesystem::path path(filePath);
    std::filesystem::create_directories(path.parent_path());

    json j;
    j["version"] = "1.0";
    j["bindings"] = json::array();

    for (const auto &[action, binding] : m_actionBindings) {
      json bindingJson;
      bindingJson["action"] = action;
      bindingJson["key"] = binding.key;
      bindingJson["modifiers"] = static_cast<int>(binding.modifiers);
      bindingJson["context"] = static_cast<int>(binding.context);
      if (m_actionDescriptions.find(action) != m_actionDescriptions.end()) {
        bindingJson["description"] = m_actionDescriptions[action];
      }
      j["bindings"].push_back(bindingJson);
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Failed to open keymap file for writing: " << filePath
                << std::endl;
      return false;
    }

    file << j.dump(4);
    file.close();

    std::cout << "Keymap saved to: " << filePath << std::endl;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Error saving keymap: " << e.what() << std::endl;
    return false;
  }
}

bool KeymapManager::loadDefaultKeymap() {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  m_keyBindings.clear();
  m_actionBindings.clear();
  createDefaultKeymap();

  return true;
}

bool KeymapManager::bindKey(const std::string &action, int key,
                            KeyModifier modifiers, WorkspaceContext context) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  std::string keyString = createKeyString(key, modifiers);

  // Remove old binding if exists
  if (m_actionBindings.find(action) != m_actionBindings.end()) {
    const auto &oldBinding = m_actionBindings[action];
    std::string oldKeyString =
        createKeyString(oldBinding.key, oldBinding.modifiers);
    m_keyBindings[oldBinding.context].erase(oldKeyString);
  }

  // Add new binding
  m_keyBindings[context][keyString] = action;

  KeyBinding binding;
  binding.action = action;
  binding.key = key;
  binding.modifiers = modifiers;
  binding.context = context;
  if (m_actionDescriptions.find(action) != m_actionDescriptions.end()) {
    binding.description = m_actionDescriptions[action];
  }
  m_actionBindings[action] = binding;

  return true;
}

bool KeymapManager::unbindKey(const std::string &action) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_actionBindings.find(action) == m_actionBindings.end()) {
    return false;
  }

  const auto &binding = m_actionBindings[action];
  std::string keyString = createKeyString(binding.key, binding.modifiers);
  m_keyBindings[binding.context].erase(keyString);
  m_actionBindings.erase(action);

  return true;
}

bool KeymapManager::unbindKey(int key, KeyModifier modifiers,
                              WorkspaceContext context) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  std::string keyString = createKeyString(key, modifiers);

  if (m_keyBindings[context].find(keyString) == m_keyBindings[context].end()) {
    return false;
  }

  std::string action = m_keyBindings[context][keyString];
  m_keyBindings[context].erase(keyString);
  m_actionBindings.erase(action);

  return true;
}

std::string KeymapManager::getActionForKey(int key, KeyModifier modifiers,
                                           WorkspaceContext context) const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  std::string keyString = createKeyString(key, modifiers);

  // Check context-specific first
  if (m_keyBindings.find(context) != m_keyBindings.end()) {
    if (m_keyBindings.at(context).find(keyString) !=
        m_keyBindings.at(context).end()) {
      return m_keyBindings.at(context).at(keyString);
    }
  }

  // Fallback to global
  if (context != WorkspaceContext::Global &&
      m_keyBindings.find(WorkspaceContext::Global) != m_keyBindings.end()) {
    if (m_keyBindings.at(WorkspaceContext::Global).find(keyString) !=
        m_keyBindings.at(WorkspaceContext::Global).end()) {
      return m_keyBindings.at(WorkspaceContext::Global).at(keyString);
    }
  }

  return "";
}

KeyBinding KeymapManager::getBindingForAction(const std::string &action,
                                              WorkspaceContext context) const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  (void)context;

  if (m_actionBindings.find(action) != m_actionBindings.end()) {
    return m_actionBindings.at(action);
  }

  return KeyBinding{};
}

std::vector<KeyBinding>
KeymapManager::getAllBindings(WorkspaceContext context) const {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  std::vector<KeyBinding> bindings;

  // Get context-specific bindings
  if (m_keyBindings.find(context) != m_keyBindings.end()) {
    for (const auto &[keyString, action] : m_keyBindings.at(context)) {
      if (m_actionBindings.find(action) != m_actionBindings.end()) {
        bindings.push_back(m_actionBindings.at(action));
      }
    }
  }

  // Add global bindings if not already included
  if (context != WorkspaceContext::Global &&
      m_keyBindings.find(WorkspaceContext::Global) != m_keyBindings.end()) {
    for (const auto &[keyString, action] :
         m_keyBindings.at(WorkspaceContext::Global)) {
      bool exists = false;
      for (const auto &binding : bindings) {
        if (binding.action == action) {
          exists = true;
          break;
        }
      }
      if (!exists && m_actionBindings.find(action) != m_actionBindings.end()) {
        bindings.push_back(m_actionBindings.at(action));
      }
    }
  }

  return bindings;
}

void KeymapManager::registerAction(const std::string &action,
                                   ActionCallback callback,
                                   const std::string &description) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  m_actionCallbacks[action] = callback;
  if (!description.empty()) {
    m_actionDescriptions[action] = description;
  }
}

void KeymapManager::unregisterAction(const std::string &action) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  m_actionCallbacks.erase(action);
}

bool KeymapManager::handleKeyPress(int key, KeyModifier modifiers,
                                   WorkspaceContext currentContext) {
  std::string action = getActionForKey(key, modifiers, currentContext);

  if (action.empty()) {
    return false;
  }

  std::lock_guard<std::recursive_mutex> lock(m_mutex);

  if (m_actionCallbacks.find(action) != m_actionCallbacks.end()) {
    m_actionCallbacks[action]();
    return true;
  }

  return false;
}

bool KeymapManager::handleKeyRelease(int key, KeyModifier modifiers,
                                     WorkspaceContext currentContext) {
  // Key release handling can be implemented if needed
  return false;
}

std::string KeymapManager::keyToString(int key) const {
  // GLFW key codes to string
  if (key >= 'A' && key <= 'Z') {
    return std::string(1, static_cast<char>(key));
  }
  if (key >= '0' && key <= '9') {
    return std::string(1, static_cast<char>(key));
  }

  // Special keys
  switch (key) {
  case 32:
    return "Space";
  case 256:
    return "Escape";
  case 257:
    return "Enter";
  case 258:
    return "Tab";
  case 259:
    return "Backspace";
  case 260:
    return "Insert";
  case 261:
    return "Delete";
  case 262:
    return "Right";
  case 263:
    return "Left";
  case 264:
    return "Down";
  case 265:
    return "Up";
  case 266:
    return "PageUp";
  case 267:
    return "PageDown";
  case 268:
    return "Home";
  case 269:
    return "End";
  default:
    return "Key" + std::to_string(key);
  }
}

std::string KeymapManager::modifiersToString(KeyModifier modifiers) const {
  std::string result;

  if (modifiers & KeyModifier::Ctrl) {
    result += "Ctrl+";
  }
  if (modifiers & KeyModifier::Shift) {
    result += "Shift+";
  }
  if (modifiers & KeyModifier::Alt) {
    result += "Alt+";
  }
  if (modifiers & KeyModifier::Super) {
    result += "Super+";
  }

  if (!result.empty() && result.back() == '+') {
    result.pop_back();
  }

  return result;
}

std::string KeymapManager::getKeyBindingString(const std::string &action,
                                               WorkspaceContext context) const {
  KeyBinding binding = getBindingForAction(action, context);
  if (binding.action.empty()) {
    return "";
  }

  std::string mods = modifiersToString(binding.modifiers);
  std::string key = keyToString(binding.key);

  if (!mods.empty()) {
    return mods + "+" + key;
  }
  return key;
}

std::string KeymapManager::createKeyString(int key,
                                           KeyModifier modifiers) const {
  return std::to_string(key) + "_" +
         std::to_string(static_cast<int>(modifiers));
}

} // namespace aether
