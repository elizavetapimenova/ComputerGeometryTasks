#include "DrawingWidget.h"
#include "PlaneGeometry/Geometry.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <cmath>

DrawingWidget::DrawingWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(250, 250, 250));
    setPalette(pal);
}

QSize DrawingWidget::minimumSizeHint() const
{
    return QSize(300, 300);
}

QSize DrawingWidget::sizeHint() const
{
    return QSize(700, 450);
}

void DrawingWidget::reset()
{
    m_points.clear();
    m_hull.clear();
    m_dragIndex.reset();
    update();
    emit hullChanged(0, 0);
}

void DrawingWidget::setHullVisible(bool visible)
{
    m_showHull = visible;
    if (m_showHull)
        recomputeHull();
    update();
}

void DrawingWidget::recomputeHull()
{
    if (!m_showHull) {
        m_hull.clear();
        emit hullChanged(m_points.size(), 0);
        return;
    }

    m_hull = PlaneGeometry::convexHull(m_points);
    emit hullChanged(m_points.size(), m_hull.size());
}

int DrawingWidget::findPointAtPosition(const QPointF& pos, double radius) const
{
    const double r2 = radius * radius;
    for (int i = 0; i < static_cast<int>(m_points.size()); ++i) {
        double dx = static_cast<double>(m_points[i].x) - pos.x();
        double dy = static_cast<double>(m_points[i].y) - pos.y();
        if (dx*dx + dy*dy <= r2)
            return i;
    }
    return -1;
}

void DrawingWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Фоновая сетка
    painter.save();
    QPen gridPen;
    gridPen.setStyle(Qt::DotLine);
    painter.setPen(gridPen);
    const int step = 25;
    for (int x = 0; x < width(); x += step)
        painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += step)
        painter.drawLine(0, y, width(), y);
    painter.restore();

    // Выпуклая оболочка
    if (m_showHull && m_hull.size() >= 2) {
        painter.save();
        QPen hullPen;
        hullPen.setWidth(2);
        painter.setPen(hullPen);
        QBrush hullBrush;
        hullBrush.setStyle(Qt::NoBrush);
        painter.setBrush(hullBrush);

        for (std::size_t i = 0; i < m_hull.size(); ++i) {
            const auto& p1 = m_hull[i];
            const auto& p2 = m_hull[(i + 1) % m_hull.size()];
            painter.drawLine(
                QPointF(static_cast<double>(p1.x), static_cast<double>(p1.y)),
                QPointF(static_cast<double>(p2.x), static_cast<double>(p2.y))
            );
        }
        painter.restore();
    }

    // Точки
    painter.save();
    QBrush pointBrush;
    pointBrush.setStyle(Qt::SolidPattern);
    painter.setBrush(pointBrush);
    painter.setPen(Qt::NoPen);

    for (const auto& p : m_points) {
        painter.drawEllipse(
            QPointF(static_cast<double>(p.x), static_cast<double>(p.y)), 4.5, 4.5);
    }
    painter.restore();
}

void DrawingWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF pos = event->position();
#else
    QPointF pos = event->pos();
#endif

    int idx = findPointAtPosition(pos, 8.0);
    if (idx >= 0) {
        // Начинаем перетаскивание существующей точки
        m_dragIndex = idx;
        m_dragOffset = QPointF(
            static_cast<double>(m_points[idx].x) - pos.x(),
            static_cast<double>(m_points[idx].y) - pos.y()
        );
        setCursor(Qt::ClosedHandCursor);
    } else {
        // Добавляем новую точку
        PlaneGeometry::Point2D p{
            static_cast<PlaneGeometry::Real>(pos.x()),
            static_cast<PlaneGeometry::Real>(pos.y())
        };
        m_points.push_back(p);
        recomputeHull();
        update();
        setCursor(Qt::ArrowCursor);
    }
}

void DrawingWidget::mouseMoveEvent(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF pos = event->position();
#else
    QPointF pos = event->pos();
#endif

    if (m_dragIndex.has_value()) {
        int idx = *m_dragIndex;
        m_points[idx].x = static_cast<PlaneGeometry::Real>(pos.x() + m_dragOffset.x());
        m_points[idx].y = static_cast<PlaneGeometry::Real>(pos.y() + m_dragOffset.y());
        recomputeHull();
        update();
    } else {
        // Меняем курсор, если навели на точку
        int idx = findPointAtPosition(pos, 8.0);
        if (idx >= 0) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void DrawingWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_dragIndex.has_value()) {
        m_dragIndex.reset();
        setCursor(Qt::ArrowCursor);
        recomputeHull();
        update();
    }
}
