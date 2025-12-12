#pragma once

#include <QWidget>
#include <vector>
#include <optional>
#include "PlaneGeometry/Point2D.h"

class DrawingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawingWidget(QWidget* parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

public slots:
    void reset();
    void setHullVisible(bool visible);

signals:
    /// Сообщает о количестве всех точек и количестве точек на оболочке
    void hullChanged(std::size_t pointCount, std::size_t hullCount);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    std::vector<PlaneGeometry::Point2D> m_points;
    std::vector<PlaneGeometry::Point2D> m_hull;
    bool m_showHull = false;

    // Для перетаскивания точек
    std::optional<int> m_dragIndex;
    QPointF            m_dragOffset;

    void recomputeHull();
    int findPointAtPosition(const QPointF& pos, double radius) const;
};
