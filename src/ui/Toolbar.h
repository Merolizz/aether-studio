#pragma once

#include "../../include/aether/Tool.h"
#include "../../include/aether/WorkspaceManager.h"
#include <vector>

namespace aether {

class Toolbar {
public:
    Toolbar();
    ~Toolbar() = default;

    Toolbar(const Toolbar&) = delete;
    Toolbar& operator=(const Toolbar&) = delete;

    void render();
    void setWorkspace(WorkspaceType workspace);
    
    // Tool management
    void activateTool(ToolType toolType);
    ToolType getActiveTool() const { return m_activeTool; }
    
    // Tool list for current workspace
    std::vector<ToolType> getToolsForWorkspace(WorkspaceType workspace) const;

private:
    void renderToolButton(ToolType toolType);
    void loadWorkspaceTools(WorkspaceType workspace);

    WorkspaceType m_currentWorkspace = WorkspaceType::Edit;
    ToolType m_activeTool = ToolType::Selection;
    std::vector<ToolType> m_currentTools;
};

} // namespace aether
