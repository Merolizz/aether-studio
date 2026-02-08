#pragma once

#include <QGraphicsView>
#include "aether/NodeGraphModel.h"

namespace aether {

class NodeGraphModel;

class NodeGraphView : public QGraphicsView {
    Q_OBJECT
public:
    explicit NodeGraphView(QWidget* parent = nullptr);
    void setModel(NodeGraphModel* model);
    NodeGraphModel* model() const { return m_model; }
    uint32_t selectedNodeId() const { return m_selectedNodeId; }

signals:
    void nodeSelected(uint32_t nodeId);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private slots:
    void onSelectionChanged();
    void onModelChanged();

private:
    void rebuildScene();
    void addNodeAt(const QPointF& scenePos, NodeType type);

    NodeGraphModel* m_model = nullptr;
    QGraphicsScene* m_scene = nullptr;
    uint32_t m_selectedNodeId = 0;
};

} // namespace aether
