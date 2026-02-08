#include "aether/ProjectFile.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QStringConverter>

namespace aether {

bool ProjectFile::saveToPath(const QString& path, const QString& projectName,
                              const QString& saveLocation, const ProjectSettings& settings) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "ProjectName=" << projectName << "\n";
    out << "SaveLocation=" << saveLocation << "\n";
    out << "Width=" << settings.width << "\n";
    out << "Height=" << settings.height << "\n";
    out << "Fps=" << settings.fps << "\n";
    out << "BitrateKbps=" << settings.bitrateKbps << "\n";
    out << "AudioSampleRate=" << settings.audioSampleRate << "\n";
    f.close();
    return true;
}

static QString readIniLine(const QString& path, const QString& key) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0) continue;
        if (line.left(eq).trimmed() == key)
            return line.mid(eq + 1).trimmed();
    }
    return QString();
}

bool ProjectFile::loadFromPath(const QString& path, QString* outProjectName,
                               QString* outSaveLocation, ProjectSettings* outSettings) {
    if (!outProjectName && !outSaveLocation && !outSettings)
        return true;
    if (outProjectName)
        *outProjectName = readIniLine(path, QStringLiteral("ProjectName"));
    if (outSaveLocation)
        *outSaveLocation = readIniLine(path, QStringLiteral("SaveLocation"));
    if (outSettings) {
        outSettings->width = readIniLine(path, QStringLiteral("Width")).toInt();
        if (outSettings->width <= 0) outSettings->width = 1920;
        outSettings->height = readIniLine(path, QStringLiteral("Height")).toInt();
        if (outSettings->height <= 0) outSettings->height = 1080;
        outSettings->fps = readIniLine(path, QStringLiteral("Fps")).toInt();
        if (outSettings->fps <= 0) outSettings->fps = 30;
        outSettings->bitrateKbps = readIniLine(path, QStringLiteral("BitrateKbps")).toInt();
        if (outSettings->bitrateKbps <= 0) outSettings->bitrateKbps = 25000;
        outSettings->audioSampleRate = readIniLine(path, QStringLiteral("AudioSampleRate")).toInt();
        if (outSettings->audioSampleRate <= 0) outSettings->audioSampleRate = 48000;
    }
    return true;
}

} // namespace aether
