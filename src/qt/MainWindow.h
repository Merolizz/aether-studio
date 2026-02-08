#pragma once

#include <QMainWindow>
#include <QScopedPointer>
#include <QVulkanInstance>
#include "aether/ProjectSettings.h"

class QToolBar;
class QWidget;
class QSplitter;
class QDockWidget;
class QStackedWidget;
class QKeyEvent;

namespace aether {

class ProjectModel;
class ProjectPanel;
class MonitorWidget;
class PlaybackEngine;
class TimelineWidget;
class AetherRenderView;
class PageBarWidget;
class MediaPageWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    enum class AppPage { Media = 0, Edit = 1, Animation = 2, Color = 3, Audio = 4, Deliver = 5 };

    enum class AppState { Home, Project };

private slots:
    void onSettings();
    void onNewProject();
    void onImportMedia();
    void onOpenProjectPath(const QString& path);
    void onExport();
    void onAddToTimeline(const QString& mediaPath);
    void onInterpretFootageRequested(const QString& mediaPath);
    void onPlayheadMoved(qint64 ms);
    void onPlaybackPositionChanged(qint64 engineMs);
    void onClipSelected(int trackIndex, int clipIndex);
    void onToolTriggered();
    void onShowPanelProject();
    void onShowPanelProperties();
    void onShowPanelEffects();
    void onShowPanelTimeline();
    void onPageChanged(int index);
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onClipEnable();
    void onClipLinkUnlink();
    void onClipVideoOptions();
    void onClipAudioOptions();
    void onSequenceAddTrack();
    void onSequenceDeleteTrack();
    void onSequenceRenderInToOut();
    void onAddMarker();
    void onNextMarker();
    void onPrevMarker();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupDarkTheme();
    void setupMenuBar();
    void setupToolsToolbar();
    void setupPanels();
    void setupCentralArea();
    void setupPageNavigation();
    void setupLicenseStatus();
    void doImportMedia();
    void doImportMediaPaths(const QStringList& paths);
    void updateMonitorForPlayhead();
    void refreshEditPanelsForClipType();
    QString resolveProxyPath(const QString& sourcePath) const;
    void enterHomeState();
    void enterProjectState();
    void appendToRecentProjects(const QString& path);

    enum class EditClipType { None, Video, Audio, Photo };
    EditClipType selectedClipType() const;

    QScopedPointer<QVulkanInstance> m_vulkanInstance;
    QScopedPointer<ProjectModel> m_projectModel;
    ProjectPanel* m_projectPanel = nullptr;
    MonitorWidget* m_monitor = nullptr;
    QScopedPointer<PlaybackEngine> m_playbackEngine;
    TimelineWidget* m_timeline = nullptr;
    QToolBar* m_toolsToolbar = nullptr;
    QSplitter* m_centralSplitter = nullptr;
    QDockWidget* m_projectDock = nullptr;
    QDockWidget* m_propertiesDock = nullptr;
    QDockWidget* m_effectsDock = nullptr;
    QStackedWidget* m_propertiesStack = nullptr;
    QStackedWidget* m_effectsStack = nullptr;
    AetherRenderView* m_renderView = nullptr;
    QWidget* m_renderContainer = nullptr;

    QStackedWidget* m_stackedPages = nullptr;
    PageBarWidget* m_pageBar = nullptr;
    MediaPageWidget* m_mediaPage = nullptr;
    int m_currentPage = 1;
    int m_lastSelectedTrack = -1;
    int m_lastSelectedClip = -1;
    QStackedWidget* m_editPropertiesStack = nullptr;
    QStackedWidget* m_editEffectsStack = nullptr;
    ProjectSettings m_projectSettings;
    AppState m_appState = AppState::Home;
    QString m_currentProjectPath;
    qint64 m_currentMonitorClipTimelineStartMs = -1;
    qint64 m_currentMonitorClipSourceInMs = 0;
    double m_currentMonitorClipSpeedRatio = 1.0;
};

} // namespace aether
