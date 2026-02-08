#pragma once

#include <QWidget>
#include <QUrl>

class QVideoWidget;
class QMediaPlayer;
class QLabel;
class QToolBar;
class QSlider;
class QTimer;
class QKeyEvent;
class QAction;

namespace aether {

class PlaybackEngine;

class MonitorWidget : public QWidget {
    Q_OBJECT
public:
    explicit MonitorWidget(QWidget* parent = nullptr);
    ~MonitorWidget() override;

    void setSource(const QUrl& url);
    void setSource(const QString& path);
    void clearSource();
    void setPlaybackEngine(PlaybackEngine* engine);
    qint64 positionMs() const;
    qint64 durationMs() const;
    void setPositionMs(qint64 ms);
    void setTimecodeFromSequence(qint64 ms);
    void setProjectFps(int fps);

    /** J-K-L navigation: call from MainWindow when EDIT page has focus. J=rewind, K=stop, L=play (2x on repeat). */
    void onJKL(int key);

signals:
    void positionChanged(qint64 ms);
    void durationChanged(qint64 ms);
    void inPointSet(qint64 ms);
    void outPointSet(qint64 ms);
    void playbackError(const QString& message);

public slots:
    void play();
    void pause();
    void stop();
    void stepForward();
    void stepBack();
    void goToIn();
    void goToOut();
    void setInPoint();
    void setOutPoint();

private slots:
    void onPositionChanged(qint64 pos);
    void onDurationChanged(qint64 dur);
    void onMediaStatusChanged();
    void onPlaybackStateChanged();
    void onEngineFrameReady();
    void onEnginePositionChanged(qint64 ms);
    void onEngineDurationChanged(qint64 ms);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void updateTimecodeLabel();
    void stopJKLScrub();
    QString msToTimecode(qint64 ms) const;

    QWidget* m_videoContainer = nullptr;
    QVideoWidget* m_videoWidget = nullptr;
    QLabel* m_engineFrameLabel = nullptr;
    QMediaPlayer* m_player = nullptr;
    PlaybackEngine* m_playbackEngine = nullptr;
    QLabel* m_timecodeLabel = nullptr;
    QLabel* m_durationLabel = nullptr;
    QToolBar* m_transportBar = nullptr;
    QSlider* m_seekSlider = nullptr;
    qint64 m_inPointMs = 0;
    qint64 m_outPointMs = 0;
    qint64 m_sequenceTimeMs = 0;
    bool m_useSequenceTime = false;

    QTimer* m_jklScrubTimer = nullptr;
    int m_jklScrubStepMs = 0;
    int m_jklForwardSpeed = 1;
    QAction* m_playAction = nullptr;
    int m_projectFps = 30;
};

} // namespace aether
