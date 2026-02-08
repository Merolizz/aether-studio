#pragma once

#include "../../include/aether/WorkspaceManager.h"
#include "Toolbar.h"
#include <memory>

namespace aether {

class WorkspaceUI {
public:
    WorkspaceUI();
    ~WorkspaceUI() = default;

    WorkspaceUI(const WorkspaceUI&) = delete;
    WorkspaceUI& operator=(const WorkspaceUI&) = delete;

    void render();

private:
    void renderWorkspaceTabs();
    void renderWorkspaceContent();

    std::unique_ptr<Toolbar> m_toolbar;
};

} // namespace aether
