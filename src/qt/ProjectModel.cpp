#include "ProjectModel.h"
#include <QFileInfo>

namespace aether {

ProjectModel::ProjectModel(QObject* parent) : QObject(parent) {
    addTrack(true);
    addTrack(false);
}

qint64 ProjectModel::sequenceDurationMs() const {
    qint64 end = 0;
    for (const Track& t : m_tracks) {
        for (const TimelineClip& c : t.clips) {
            qint64 span = (c.speedRatio > 0.001)
                ? static_cast<qint64>((c.sourceOutMs - c.sourceInMs) / c.speedRatio)
                : (c.sourceOutMs - c.sourceInMs);
            qint64 clipEnd = c.timelineStartMs + span;
            if (clipEnd > end) end = clipEnd;
        }
    }
    return end;
}

int ProjectModel::addMedia(const QString& path, const QString& name, qint64 durationMs, bool isVideo, bool isAudio) {
    MediaItem item;
    item.path = path;
    item.name = name.isEmpty() ? QFileInfo(path).fileName() : name;
    item.durationMs = durationMs;
    item.isVideo = isVideo;
    item.isAudio = isAudio;
    m_media.append(item);
    emit mediaListChanged();
    return m_media.size() - 1;
}

void ProjectModel::setMediaMetadata(int mediaIndex, int width, int height, int fps) {
    if (mediaIndex < 0 || mediaIndex >= m_media.size()) return;
    m_media[mediaIndex].width = width;
    m_media[mediaIndex].height = height;
    m_media[mediaIndex].fps = fps;
    emit mediaListChanged();
}

void ProjectModel::setMediaMetadataByPath(const QString& path, int width, int height, int fps) {
    for (int i = 0; i < m_media.size(); i++) {
        if (m_media[i].path == path) {
            setMediaMetadata(i, width, height, fps);
            return;
        }
    }
}

int ProjectModel::mediaInterpretFps(const QString& path) const {
    for (const MediaItem& m : m_media) {
        if (m.path == path) return m.interpretFps;
    }
    return 0;
}

void ProjectModel::clearMedia() {
    m_media.clear();
    emit mediaListChanged();
}

void ProjectModel::addTrack(bool isVideo) {
    Track t;
    t.name = isVideo ? QString("Video %1").arg(m_tracks.size() + 1) : QString("Audio %1").arg(m_tracks.size() + 1);
    t.isVideo = isVideo;
    m_tracks.append(t);
    emit tracksChanged();
}

void ProjectModel::removeTrack(int index) {
    if (index < 0 || index >= m_tracks.size()) return;
    m_tracks.removeAt(index);
    emit tracksChanged();
}

void ProjectModel::addClipToTrack(int trackIndex, const QString& mediaPath, qint64 sourceInMs, qint64 sourceOutMs, qint64 timelineStartMs) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    TimelineClip c;
    c.mediaPath = mediaPath;
    c.sourceInMs = sourceInMs;
    c.sourceOutMs = sourceOutMs;
    c.timelineStartMs = timelineStartMs;
    c.trackIndex = trackIndex;
    m_tracks[trackIndex].clips.append(c);
    emit tracksChanged();
}

void ProjectModel::removeClip(int trackIndex, int clipIndex) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    auto& clips = m_tracks[trackIndex].clips;
    if (clipIndex < 0 || clipIndex >= clips.size()) return;
    clips.removeAt(clipIndex);
    emit tracksChanged();
}

void ProjectModel::clearAllClips() {
    for (Track& t : m_tracks)
        t.clips.clear();
    emit tracksChanged();
}

void ProjectModel::moveClip(int fromTrack, int fromClip, int toTrack, qint64 newStartMs) {
    if (fromTrack < 0 || fromTrack >= m_tracks.size() || toTrack < 0 || toTrack >= m_tracks.size()) return;
    auto& src = m_tracks[fromTrack].clips;
    if (fromClip < 0 || fromClip >= src.size()) return;
    TimelineClip c = src.takeAt(fromClip);
    c.timelineStartMs = newStartMs;
    c.trackIndex = toTrack;
    m_tracks[toTrack].clips.append(c);
    emit tracksChanged();
}

void ProjectModel::setClipInOut(int trackIndex, int clipIndex, qint64 sourceInMs, qint64 sourceOutMs) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    auto& clips = m_tracks[trackIndex].clips;
    if (clipIndex < 0 || clipIndex >= clips.size()) return;
    clips[clipIndex].sourceInMs = sourceInMs;
    clips[clipIndex].sourceOutMs = sourceOutMs;
    emit tracksChanged();
}

void ProjectModel::setClipStart(int trackIndex, int clipIndex, qint64 timelineStartMs) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    auto& clips = m_tracks[trackIndex].clips;
    if (clipIndex < 0 || clipIndex >= clips.size()) return;
    clips[clipIndex].timelineStartMs = timelineStartMs;
    emit tracksChanged();
}

void ProjectModel::setClipSpeedRatio(int trackIndex, int clipIndex, double speedRatio) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    auto& clips = m_tracks[trackIndex].clips;
    if (clipIndex < 0 || clipIndex >= clips.size()) return;
    if (speedRatio < 0.01) speedRatio = 0.01;
    if (speedRatio > 100.0) speedRatio = 100.0;
    clips[clipIndex].speedRatio = speedRatio;
    emit tracksChanged();
}

void ProjectModel::setClipScaleToFrame(int trackIndex, int clipIndex, bool scaleToFrame) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return;
    auto& clips = m_tracks[trackIndex].clips;
    if (clipIndex < 0 || clipIndex >= clips.size()) return;
    clips[clipIndex].scaleToFrame = scaleToFrame;
    emit tracksChanged();
}

void ProjectModel::setMediaInterpretFpsByPath(const QString& path, int interpretFps) {
    for (MediaItem& m : m_media) {
        if (m.path == path) {
            m.interpretFps = interpretFps >= 0 ? interpretFps : 0;
            emit mediaListChanged();
            return;
        }
    }
}

bool ProjectModel::splitClipAt(int trackIndex, qint64 positionMs, int* outTrackIndex, int* outClipIndex) {
    if (trackIndex < 0 || trackIndex >= m_tracks.size()) return false;
    auto& clips = m_tracks[trackIndex].clips;
    for (int i = 0; i < clips.size(); ++i) {
        const TimelineClip& c = clips[i];
        qint64 start = c.timelineStartMs;
        qint64 end = c.timelineStartMs + (c.sourceOutMs - c.sourceInMs);
        if (positionMs > start && positionMs < end) {
            qint64 dur = c.sourceOutMs - c.sourceInMs;
            qint64 leftDur = positionMs - start;
            (void)(dur - leftDur);
            TimelineClip right = c;
            right.sourceInMs = c.sourceInMs + leftDur;
            right.timelineStartMs = positionMs;
            clips[i].sourceOutMs = c.sourceInMs + leftDur;
            clips.insert(i + 1, right);
            if (outTrackIndex) *outTrackIndex = trackIndex;
            if (outClipIndex) *outClipIndex = i + 1;
            emit tracksChanged();
            return true;
        }
    }
    return false;
}

} // namespace aether
