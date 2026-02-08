#pragma once

#include <QWidget>
#include <memory>

namespace aether {

class NodeGraphModel;
class KeyframeModel;
class NodeGraphView;
class KeyframeTimelineWidget;

class AnimationPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit AnimationPageWidget(QWidget* parent = nullptr);
    ~AnimationPageWidget() override;

private slots:
    void onNodeSelected(uint32_t nodeId);

private:
    void buildDefaultParametersForNode(uint32_t nodeId);

    std::unique_ptr<NodeGraphModel> m_nodeGraph;
    std::unique_ptr<KeyframeModel> m_keyframeModel;
    NodeGraphView* m_nodeGraphView = nullptr;
    KeyframeTimelineWidget* m_keyframeTimeline = nullptr;
};

} // namespace aether
