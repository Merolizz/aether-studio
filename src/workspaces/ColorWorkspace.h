#pragma once

#include "../../include/aether/WorkspaceManager.h"
#include <string>
#include <memory>

namespace aether {

class ColorWorkspace : public Workspace {
public:
    ColorWorkspace();
    virtual ~ColorWorkspace() = default;

    // Lifecycle
    void activate() override;
    void deactivate() override;
    void update(float deltaTime) override;
    void render() override;
    
    // Tool management
    void loadTools() override;
    void unloadTools() override;

private:
    bool m_toolsLoaded = false;
};

} // namespace aether
