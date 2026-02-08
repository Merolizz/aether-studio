#pragma once

#include <QObject>
#include <QString>
#include <QImage>
#include <QTimer>
#include <memory>
#include <atomic>
#include <cstdint>

namespace aether {

class FrameRingBuffer;

class PlaybackEngine : public QObject {
    Q_OBJECT
public:
    explicit PlaybackEngine(QObject* parent = nullptr);
    ~PlaybackEngine() override;

    void setSource(const QString& path);
    void play();
    void pause();
    void stop();
    void seek(qint64 positionMs);

    qint64 getCurrentTimeMs() const;
    qint64 getDurationMs() const;
    bool hasSource() const { return !m_sourcePath.isEmpty(); }
    QImage getCurrentFrame() const;

signals:
    void positionChanged(qint64 ms);
    void durationChanged(qint64 ms);
    void frameReady();
    void errorOccurred(const QString& message);

private:
    void openAndStartDecode();
    void stopDecode();
    void pollFrame();

    QString m_sourcePath;
    qint64 m_durationMs = 0;
    std::atomic<qint64> m_currentTimeMs{0};
    std::atomic<bool> m_playing{false};
    std::atomic<bool> m_seekRequested{false};
    std::atomic<qint64> m_seekTargetMs{0};
    std::unique_ptr<FrameRingBuffer> m_ringBuffer;
    class DecodeThread* m_decodeThread = nullptr;
    QTimer* m_pollTimer = nullptr;
    mutable bool m_durationEmitted = false;
};

} // namespace aether
