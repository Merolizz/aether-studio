#include "../../include/aether/UndoRedo.h"
#include <algorithm>

namespace aether {

UndoRedoManager& UndoRedoManager::getInstance() {
    static UndoRedoManager instance;
    return instance;
}

void UndoRedoManager::pushAction(std::unique_ptr<UndoRedoAction> action) {
    // Remove any actions after current index (when we're in the middle of history)
    if (m_currentIndex < m_actions.size()) {
        m_actions.erase(m_actions.begin() + m_currentIndex, m_actions.end());
    }
    
    // Add new action
    m_actions.push_back(std::move(action));
    m_currentIndex = m_actions.size();
    
    // Limit history size
    if (m_actions.size() > m_maxHistorySize) {
        m_actions.erase(m_actions.begin());
        m_currentIndex--;
    }
}

bool UndoRedoManager::undo() {
    if (!canUndo()) {
        return false;
    }
    
    m_currentIndex--;
    m_actions[m_currentIndex]->undo();
    return true;
}

bool UndoRedoManager::redo() {
    if (!canRedo()) {
        return false;
    }
    
    m_actions[m_currentIndex]->redo();
    m_currentIndex++;
    return true;
}

void UndoRedoManager::clear() {
    m_actions.clear();
    m_currentIndex = 0;
}

std::string UndoRedoManager::getUndoDescription() const {
    if (!canUndo()) {
        return "";
    }
    return m_actions[m_currentIndex - 1]->getDescription();
}

std::string UndoRedoManager::getRedoDescription() const {
    if (!canRedo()) {
        return "";
    }
    return m_actions[m_currentIndex]->getDescription();
}

} // namespace aether
