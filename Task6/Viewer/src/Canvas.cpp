#include "Canvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <vector>
#include <algorithm>

// Объявление внешней функции
std::vector<Triangle> delaunayTriangulation(const std::vector<Point>& points);

DrawingWidget::DrawingWidget(QWidget *parent) : QWidget(parent), draggingIndex(-1) {
    setMouseTracking(true);
}

void DrawingWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Рисуем треугольники
    painter.setPen(QPen(Qt::blue, 1));
    painter.setBrush(Qt::NoBrush);

    for (const auto &t : triangles) {
        painter.drawLine(QPointF(t.a.x, t.a.y), QPointF(t.b.x, t.b.y));
        painter.drawLine(QPointF(t.b.x, t.b.y), QPointF(t.c.x, t.c.y));
        painter.drawLine(QPointF(t.c.x, t.c.y), QPointF(t.a.x, t.a.y));
    }

    // Рисуем точки
    painter.setPen(Qt::black);
    painter.setBrush(Qt::red);

    for (const auto &p : points) {
        painter.drawEllipse(QPointF(p.x, p.y), 5, 5);
    }

    // Выделяем перетаскиваемую точку
    if (draggingIndex >= 0 && draggingIndex < points.size()) {
        painter.setBrush(Qt::green);
        painter.drawEllipse(QPointF(points[draggingIndex].x, points[draggingIndex].y), 7, 7);
    }
}

void DrawingWidget::mousePressEvent(QMouseEvent* event) {
    QPointF pos = event->position();
    draggingIndex = -1;

    // Проверяем, кликнули ли на существующую точку
    for (int i = 0; i < points.size(); ++i) {
        double dx = points[i].x - pos.x();
        double dy = points[i].y - pos.y();
        if (dx*dx + dy*dy < 64.0) { // 8 пикселей в радиусе
            draggingIndex = i;
            break;
        }
    }

    // Если не кликнули на точку, добавляем новую
    if (draggingIndex == -1) {
        if (event->button() == Qt::LeftButton) {
            points.append(Point(pos));
            if (autoUpdate) rebuildTriangulation();
            update();
        }
    }
}

void DrawingWidget::mouseMoveEvent(QMouseEvent* event) {
    if (draggingIndex != -1 && draggingIndex < points.size()) {
        QPointF pos = event->position();
        points[draggingIndex] = Point(pos);
        if (autoUpdate) rebuildTriangulation();
        update();
    }
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && draggingIndex != -1 && !autoUpdate) {
        rebuildTriangulation();
    }
    draggingIndex = -1;
}

void DrawingWidget::rebuildTriangulation() {
    if (points.size() < 3) {
        triangles.clear();
        update();
        return;
    }

    // Конвертируем в std::vector
    std::vector<Point> stdPoints;
    stdPoints.reserve(points.size());
    for (const auto &p : points) {
        stdPoints.push_back(p);
    }

    // Вызываем алгоритм триангуляции
    std::vector<Triangle> temp = delaunayTriangulation(stdPoints);

    // Конвертируем обратно в QVector
    triangles.clear();
    triangles.reserve(static_cast<int>(temp.size()));
    for (const auto &t : temp) {
        triangles.append(Triangle(t));
    }

    update();
}

void DrawingWidget::clearPoints() {  // ЭТА ФУНКЦИЯ ДОЛЖНА БЫТЬ!
    points.clear();
    triangles.clear();
    draggingIndex = -1;
    update();
}
