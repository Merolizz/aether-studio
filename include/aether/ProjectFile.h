#pragma once

#include "ProjectSettings.h"
#include <QString>

namespace aether {

class ProjectFile {
public:
    static bool saveToPath(const QString& path, const QString& projectName,
                           const QString& saveLocation, const ProjectSettings& settings);
    static bool loadFromPath(const QString& path, QString* outProjectName,
                             QString* outSaveLocation, ProjectSettings* outSettings);
};

} // namespace aether
