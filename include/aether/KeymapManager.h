#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace aether {

enum class KeyModifier {
    None = 0,
    Ctrl = 1 << 0,
    Shift = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3  // Windows key / Cmd
};

inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(KeyModifier a, KeyModifier b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

// Key binding contexts. Named differently to avoid clashing with `aether::Workspace`
// class in WorkspaceManager.h.
enum class WorkspaceContext {
    Global,
    Edit,
    Animation,
    Photo,
    Color,
    Audio,
    Deliver
};

struct KeyBinding {
    std::string action;
    int key = 0;
    KeyModifier modifiers = KeyModifier::None;
    WorkspaceContext context = WorkspaceContext::Global;
    std::string description;
};

class KeymapManager {
public:
    using ActionCallback = std::function<void()>;
    
    // Singleton access
    static KeymapManager& getInstance();
    
    // Delete copy constructor and assignment operator
    KeymapManager(const KeymapManager&) = delete;
    KeymapManager& operator=(const KeymapManager&) = delete;

    // Initialization
    bool initialize();
    void shutdown();

    // Keymap loading/saving
    bool loadKeymap(const std::string& filePath);
    bool saveKeymap(const std::string& filePath);
    bool loadDefaultKeymap();
    
    // Key binding management
    bool bindKey(const std::string& action, int key, KeyModifier modifiers = KeyModifier::None, WorkspaceContext context = WorkspaceContext::Global);
    bool unbindKey(const std::string& action);
    bool unbindKey(int key, KeyModifier modifiers, WorkspaceContext context);
    
    // Key lookup
    std::string getActionForKey(int key, KeyModifier modifiers, WorkspaceContext context) const;
    KeyBinding getBindingForAction(const std::string& action, WorkspaceContext context) const;
    std::vector<KeyBinding> getAllBindings(WorkspaceContext context = WorkspaceContext::Global) const;
    
    // Action registration
    void registerAction(const std::string& action, ActionCallback callback, const std::string& description = "");
    void unregisterAction(const std::string& action);
    
    // Key event handling
    bool handleKeyPress(int key, KeyModifier modifiers, WorkspaceContext currentContext);
    bool handleKeyRelease(int key, KeyModifier modifiers, WorkspaceContext currentContext);
    
    // Utility
    std::string keyToString(int key) const;
    std::string modifiersToString(KeyModifier modifiers) const;
    std::string getKeyBindingString(const std::string& action, WorkspaceContext context) const;
    std::string getKeymapFilePath() const;

private:
    KeymapManager() = default;
    ~KeymapManager() = default;

    void createDefaultKeymap();
    
    // Key binding storage: context -> (key+modifiers -> action)
    std::map<WorkspaceContext, std::map<std::string, std::string>> m_keyBindings;
    
    // Action -> Binding lookup
    std::map<std::string, KeyBinding> m_actionBindings;
    
    // Action callbacks
    std::map<std::string, ActionCallback> m_actionCallbacks;
    std::map<std::string, std::string> m_actionDescriptions;
    
    mutable std::recursive_mutex m_mutex;
    bool m_initialized = false;
    
    // Helper to create key string
    std::string createKeyString(int key, KeyModifier modifiers) const;
};

} // namespace aether
