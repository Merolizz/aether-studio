#include "AnimationPageWidget.h"
#include "NodeGraphView.h"
#include "KeyframeTimelineWidget.h"
#include "aether/NodeGraphModel.h"
#include "aether/KeyframeModel.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

AnimationPageWidget::AnimationPageWidget(QWidget* parent)
    : QWidget(parent) {
    m_nodeGraph = std::make_unique<NodeGraphModel>();
    m_keyframeModel = std::make_unique<KeyframeModel>();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter* split = new QSplitter(Qt::Vertical, this);
    m_nodeGraphView = new NodeGraphView(this);
    m_nodeGraphView->setModel(m_nodeGraph.get());
    connect(m_nodeGraphView, &NodeGraphView::nodeSelected, this, &AnimationPageWidget::onNodeSelected);
    split->addWidget(m_nodeGraphView);

    QWidget* keyframePanel = new QWidget(this);
    QVBoxLayout* kfLayout = new QVBoxLayout(keyframePanel);
    kfLayout->setContentsMargins(8, 8, 8, 8);
    QLabel* kfLabel = new QLabel(tr("Keyframe Timeline — select a node above"), keyframePanel);
    kfLabel->setStyleSheet("color: #888; font-size: 12px;");
    kfLayout->addWidget(kfLabel);
    m_keyframeTimeline = new KeyframeTimelineWidget(this);
    m_keyframeTimeline->setModel(m_keyframeModel.get());
    kfLayout->addWidget(m_keyframeTimeline, 1);
    split->addWidget(keyframePanel);
    split->setStretchFactor(0, 2);
    split->setStretchFactor(1, 1);
    split->setSizes({ 400, 200 });

    mainLayout->addWidget(split);
}

AnimationPageWidget::~AnimationPageWidget() = default;

void AnimationPageWidget::onNodeSelected(uint32_t nodeId) {
    buildDefaultParametersForNode(nodeId);
    m_keyframeTimeline->update();
}

void AnimationPageWidget::buildDefaultParametersForNode(uint32_t nodeId) {
    m_keyframeModel->setNodeId(nodeId);
    const Node* n = m_nodeGraph->nodeById(nodeId);
    if (!n) {
        m_keyframeModel->setParameters({});
        return;
    }
    std::vector<ParameterKeyframes> params;
    switch (n->type) {
        case NodeType::Transform: {
            params.push_back({ "x", "Position X", -1000, 1000, {} });
            params.push_back({ "y", "Position Y", -1000, 1000, {} });
            params.push_back({ "scale", "Scale", 0, 3, {} });
            params.push_back({ "rotation", "Rotation", 0, 360, {} });
            break;
        }
        case NodeType::Blur: {
            params.push_back({ "radius", "Blur Radius", 0, 100, {} });
            break;
        }
        case NodeType::Merge:
        case NodeType::Mask: {
            params.push_back({ "mix", "Mix", 0, 1, {} });
            break;
        }
        case NodeType::Glow: {
            params.push_back({ "intensity", "Intensity", 0, 2, {} });
            params.push_back({ "threshold", "Threshold", 0, 1, {} });
            break;
        }
    }
    m_keyframeModel->setParameters(params);
}

} // namespace aether
