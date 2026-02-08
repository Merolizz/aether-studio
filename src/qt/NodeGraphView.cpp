#include "NodeGraphView.h"
#include "aether/NodeGraphModel.h"

#include <cstddef>

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QApplication>

namespace aether {

static const int NodeWidth = 140;
static const int NodeHeaderHeight = 24;
static const int PortRadius = 5;
static const int PortSpacing = 22;

class NodeItem : public QGraphicsRectItem {
public:
    NodeItem(uint32_t id, const QString& title, int inputCount, int outputCount)
        : QGraphicsRectItem(0, 0, NodeWidth, NodeHeaderHeight + std::max(inputCount, outputCount) * PortSpacing)
        , m_id(id)
    {
        setFlag(QGraphicsItem::ItemIsMovable);
        setFlag(QGraphicsItem::ItemIsSelectable);
        setBrush(QColor(45, 45, 50));
        setPen(QPen(QColor(60, 60, 66), 1));
        m_title = new QGraphicsTextItem(title, this);
        m_title->setDefaultTextColor(QColor(230, 230, 230));
        m_title->setPos(6, 4);
        m_inputCount = inputCount;
        m_outputCount = outputCount;
    }
    uint32_t nodeIdentifier() const { return m_id; }
    int inputCount() const { return m_inputCount; }
    int outputCount() const { return m_outputCount; }
    QPointF portPosInput(int idx) const {
        return QPointF(0, NodeHeaderHeight + (idx + 1) * PortSpacing - PortRadius);
    }
    QPointF portPosOutput(int idx) const {
        return QPointF(NodeWidth, NodeHeaderHeight + (idx + 1) * PortSpacing - PortRadius);
    }

private:
    uint32_t m_id;
    QGraphicsTextItem* m_title = nullptr;
    int m_inputCount = 0;
    int m_outputCount = 0;
};

static void nodeConnectionOutputOffset(int portIndex, qreal* outOx, qreal* outOy) {
    *outOx = NodeWidth;
    *outOy = NodeHeaderHeight + (portIndex + 1) * PortSpacing - PortRadius;
}
static void nodeConnectionInputOffset(int portIndex, qreal* outOx, qreal* outOy) {
    *outOx = 0;
    *outOy = NodeHeaderHeight + (portIndex + 1) * PortSpacing - PortRadius;
}

class ConnectionLine : public QGraphicsPathItem {
public:
    ConnectionLine(const QPointF& p1, const QPointF& p2) : QGraphicsPathItem() {
        setPen(QPen(QColor(94, 129, 172), 2));
        setZValue(-1);
        updatePath(p1, p2);
    }
    void updatePath(const QPointF& p1, const QPointF& p2) {
        QPainterPath path;
        path.moveTo(p1);
        qreal dx = p2.x() - p1.x();
        path.cubicTo(p1.x() + dx * 0.5, p1.y(), p2.x() - dx * 0.5, p2.y(), p2.x(), p2.y());
        setPath(path);
    }
};

NodeGraphView::NodeGraphView(QWidget* parent)
    : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor(30, 30, 34));
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setResizeAnchor(AnchorUnderMouse);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &NodeGraphView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QPointF scenePos = mapToScene(pos);
        QMenu menu(this);
        menu.addAction(tr("Add Transform"), this, [this, scenePos]() { addNodeAt(scenePos, NodeType::Transform); });
        menu.addAction(tr("Add Blur"), this, [this, scenePos]() { addNodeAt(scenePos, NodeType::Blur); });
        menu.addAction(tr("Add Merge"), this, [this, scenePos]() { addNodeAt(scenePos, NodeType::Merge); });
        menu.addAction(tr("Add Mask"), this, [this, scenePos]() { addNodeAt(scenePos, NodeType::Mask); });
        menu.addAction(tr("Add Glow"), this, [this, scenePos]() { addNodeAt(scenePos, NodeType::Glow); });
        menu.exec(mapToGlobal(pos));
    });
}

void NodeGraphView::setModel(NodeGraphModel* model) {
    m_model = model;
    rebuildScene();
    if (m_model)
        connect(m_scene, &QGraphicsScene::selectionChanged, this, &NodeGraphView::onSelectionChanged);
}

void NodeGraphView::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->fillRect(rect, QColor(30, 30, 34));
    const int grid = 20;
    painter->setPen(QPen(QColor(50, 50, 54), 1));
    for (qreal x = rect.left(); x <= rect.right(); x += grid)
        painter->drawLine(QPointF(x, rect.top()), QPointF(x, rect.bottom()));
    for (qreal y = rect.top(); y <= rect.bottom(); y += grid)
        painter->drawLine(QPointF(rect.left(), y), QPointF(rect.right(), y));
}

void NodeGraphView::rebuildScene() {
    m_scene->clear();
    m_selectedNodeId = 0;
    if (!m_model) return;

    for (const auto& n : m_model->nodes()) {
        NodeItem* item = new NodeItem(n.id, QString::fromStdString(n.title),
            static_cast<int>(n.inputPortNames.size()), static_cast<int>(n.outputPortNames.size()));
        item->setPos(n.x, n.y);
        m_scene->addItem(item);
    }

    for (const auto& c : m_model->connections()) {
        const Node* src = m_model->nodeById(c.source.nodeId);
        const Node* dest = m_model->nodeById(c.dest.nodeId);
        if (!src || !dest) continue;
        NodeItem* srcItem = nullptr;
        NodeItem* destItem = nullptr;
        for (QGraphicsItem* it : m_scene->items()) {
            NodeItem* ni = qgraphicsitem_cast<NodeItem*>(it);
            if (ni && ni->nodeIdentifier() == src->id) srcItem = ni;
            if (ni && ni->nodeIdentifier() == dest->id) destItem = ni;
        }
        if (!srcItem || !destItem) continue;
        const int srcPort = static_cast<int>(c.source.portIndex);
        const int destPort = static_cast<int>(c.dest.portIndex);
        qreal outOx, outOy, inOx, inOy;
        nodeConnectionOutputOffset(srcPort, &outOx, &outOy);
        nodeConnectionInputOffset(destPort, &inOx, &inOy);
        const char* srcBase = reinterpret_cast<const char*>(src);
        const char* destBase = reinterpret_cast<const char*>(dest);
        float srcPx = *reinterpret_cast<const float*>(srcBase + offsetof(Node, x));
        float srcPy = *reinterpret_cast<const float*>(srcBase + offsetof(Node, y));
        float destPx = *reinterpret_cast<const float*>(destBase + offsetof(Node, x));
        float destPy = *reinterpret_cast<const float*>(destBase + offsetof(Node, y));
        qreal p1a = static_cast<qreal>(srcPx) + outOx;
        qreal p1b = static_cast<qreal>(srcPy) + outOy;
        qreal p2a = static_cast<qreal>(destPx) + inOx;
        qreal p2b = static_cast<qreal>(destPy) + inOy;
        QPointF p1{ p1a, p1b };
        QPointF p2{ p2a, p2b };
        auto* line = new ConnectionLine(p1, p2);
        m_scene->addItem(line);
        line->setZValue(-1);
    }
}

void NodeGraphView::onSelectionChanged() {
    m_selectedNodeId = 0;
    for (QGraphicsItem* it : m_scene->selectedItems()) {
        NodeItem* ni = qgraphicsitem_cast<NodeItem*>(it);
        if (ni) { m_selectedNodeId = ni->nodeIdentifier(); break; }
    }
    emit nodeSelected(m_selectedNodeId);
}

void NodeGraphView::onModelChanged() {
    rebuildScene();
}

void NodeGraphView::addNodeAt(const QPointF& scenePos, NodeType type) {
    if (!m_model) return;
    m_model->addNode(type, static_cast<float>(scenePos.x()), static_cast<float>(scenePos.y()));
    rebuildScene();
}

} // namespace aether
