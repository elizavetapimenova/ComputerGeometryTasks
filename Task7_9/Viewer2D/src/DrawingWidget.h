#pragma once
#include <QWidget>
#include <vector>
#include <optional>
#include "PlaneGeometry/Geometry.h"

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent=nullptr);
    QSize minimumSizeHint() const override { return {640,480}; }

    enum class Op { Intersect, Union, Difference };

private:
    Op m_op{Op::Intersect};

public:
    void finalizeFirst();
    void finalizeSecond();
    void clearAll();
    QPointF toScreen(const PlaneGeometry::Point& p) const;

    void setOp(Op op) { m_op = op; recomputeResult(); update(); }

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;

private:
    enum class Phase { EditingFirst, EditingSecond, Ready };
    Phase m_phase{Phase::EditingFirst};

    std::vector<PlaneGeometry::Point> m_ptsA, m_ptsB;
    PlaneGeometry::Polygon m_hullA, m_hullB;


    std::vector<PlaneGeometry::Polygon> m_result;

    PlaneGeometry::Polygon m_intersection;

    int  m_dragIndex{-1};
    bool m_dragging{false};
    bool m_pressedOnPoint{false};
    bool m_moved{false};
    bool m_suppressClick{false};
    QPoint m_pressPos;


    bool m_diffBA{false};


    PlaneGeometry::Point fromScreen(const QPointF& q) const;

    std::optional<int> hitPointIndex(const std::vector<PlaneGeometry::Point>& pts,
                                     const QPoint& pos, double tol=8.0) const;
    std::vector<PlaneGeometry::Point>& currentPts();

    void recomputeHulls();
    void recomputeResult();

    bool nearExistingPoint(const std::vector<PlaneGeometry::Point>& pts,
                           const PlaneGeometry::Point& p, double tol=0.15) const;
};
