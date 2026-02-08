#pragma once

#include <string>
#include <vector>

namespace aether {

class FileDialog {
public:
    enum class Mode {
        Open,
        Save
    };
    
    static std::string openFile(
        const std::string& title = "Open File",
        const std::vector<std::pair<std::string, std::string>>& filters = {},
        const std::string& defaultPath = ""
    );
    
    static std::string saveFile(
        const std::string& title = "Save File",
        const std::vector<std::pair<std::string, std::string>>& filters = {},
        const std::string& defaultPath = "",
        const std::string& defaultName = ""
    );
    
    static std::string openFolder(
        const std::string& title = "Select Folder",
        const std::string& defaultPath = ""
    );
};

} // namespace aether
