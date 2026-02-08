#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>

namespace aether {

class UndoRedoAction {
public:
    virtual ~UndoRedoAction() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual std::string getDescription() const = 0;
};

class UndoRedoManager {
public:
    // Singleton access
    static UndoRedoManager& getInstance();
    
    // Delete copy constructor and assignment operator
    UndoRedoManager(const UndoRedoManager&) = delete;
    UndoRedoManager& operator=(const UndoRedoManager&) = delete;

    // Action management
    void pushAction(std::unique_ptr<UndoRedoAction> action);
    bool undo();
    bool redo();
    void clear();
    
    // State queries
    bool canUndo() const { return m_currentIndex > 0; }
    bool canRedo() const { return m_currentIndex < m_actions.size(); }
    std::string getUndoDescription() const;
    std::string getRedoDescription() const;
    
    // History management
    void setMaxHistorySize(size_t size) { m_maxHistorySize = size; }
    size_t getHistorySize() const { return m_actions.size(); }

private:
    UndoRedoManager() = default;
    ~UndoRedoManager() = default;

    std::vector<std::unique_ptr<UndoRedoAction>> m_actions;
    size_t m_currentIndex = 0;
    size_t m_maxHistorySize = 100;
};

} // namespace aether
