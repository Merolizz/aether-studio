#pragma once

#include <QObject>
#include <QString>
#include <QVector>

namespace aether {

struct MediaItem {
    QString path;
    QString name;
    qint64 durationMs = 0;
    bool isVideo = true;
    bool isAudio = false;
    int width = 0;
    int height = 0;
    int fps = 0;
    int interpretFps = 0;
};

struct TimelineClip {
    QString mediaPath;
    qint64 sourceInMs = 0;
    qint64 sourceOutMs = 0;
    qint64 timelineStartMs = 0;
    int trackIndex = 0;
    double speedRatio = 1.0;
    bool scaleToFrame = false;
};

struct Track {
    QString name;
    bool isVideo = true;
    QVector<TimelineClip> clips;
};

class ProjectModel : public QObject {
    Q_OBJECT
public:
    explicit ProjectModel(QObject* parent = nullptr);

    const QVector<MediaItem>& media() const { return m_media; }
    const QVector<Track>& tracks() const { return m_tracks; }
    qint64 sequenceDurationMs() const;

    int addMedia(const QString& path, const QString& name, qint64 durationMs, bool isVideo, bool isAudio);
    void setMediaMetadata(int mediaIndex, int width, int height, int fps);
    void setMediaMetadataByPath(const QString& path, int width, int height, int fps);
    int mediaInterpretFps(const QString& path) const;
    void clearMedia();
    void addTrack(bool isVideo = true);
    void removeTrack(int index);
    void addClipToTrack(int trackIndex, const QString& mediaPath, qint64 sourceInMs, qint64 sourceOutMs, qint64 timelineStartMs);
    void removeClip(int trackIndex, int clipIndex);
    void clearAllClips();
    void moveClip(int fromTrack, int fromClip, int toTrack, qint64 newStartMs);
    void setClipInOut(int trackIndex, int clipIndex, qint64 sourceInMs, qint64 sourceOutMs);
    void setClipStart(int trackIndex, int clipIndex, qint64 timelineStartMs);
    void setClipSpeedRatio(int trackIndex, int clipIndex, double speedRatio);
    void setClipScaleToFrame(int trackIndex, int clipIndex, bool scaleToFrame);
    void setMediaInterpretFpsByPath(const QString& path, int interpretFps);
    bool splitClipAt(int trackIndex, qint64 positionMs, int* outTrackIndex, int* outClipIndex);

signals:
    void mediaListChanged();
    void tracksChanged();

private:
    QVector<MediaItem> m_media;
    QVector<Track> m_tracks;
};

} // namespace aether
