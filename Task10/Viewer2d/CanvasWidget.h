#pragma once
#include <QWidget>
#include <vector>
#include "plane_geometry/Geometry.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);
    QSize minimumSizeHint() const override { return {640, 480}; }

    enum class Op { Intersect, Union, Difference };

    void setOp(Op op);
    void finalizeFirst();
    void finalizeSecond();
    void clearAll();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    using Point   = PlaneGeometry::Point;
    using Polygon = PlaneGeometry::Polygon;

    enum class Phase { EditingFirst, EditingSecond, Ready };

    Op    m_op{Op::Intersect};
    Phase m_phase{Phase::EditingFirst};

    Polygon m_polyA;
    Polygon m_polyB;
    bool    m_closedA{false};
    bool    m_closedB{false};

    bool m_dragging{false};
    bool m_dragInA{true};
    int  m_dragIndex{-1};

    double m_hitRadiusPx{8.0};

    void addPointForCurrent(const QPointF& pos);
    bool pickVertex(const QPointF& pos);
    QPainterPath pathFromPoly(const Polygon& poly, bool closed) const;
};
