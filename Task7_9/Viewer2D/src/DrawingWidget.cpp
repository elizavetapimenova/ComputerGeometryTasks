#include "DrawingWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QMouseEvent>
#include <QStyleOption>
#include <QLinearGradient>
#include <QFontMetrics>

using PlaneGeometry::Point;
using PlaneGeometry::Polygon;

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(true);
}

void CanvasWidget::finalizeFirst() {
    if (!m_ptsA.empty()) {
        m_hullA = PlaneGeometry::convexHull(m_ptsA);
        m_phase = Phase::EditingSecond;
        recomputeResult();
        update();
    }
}

void CanvasWidget::finalizeSecond() {
    if (!m_ptsB.empty()) {
        m_hullB = PlaneGeometry::convexHull(m_ptsB);
        m_phase = Phase::Ready;
        recomputeResult();
        update();
    }
}

void CanvasWidget::clearAll() {
    m_ptsA.clear(); m_ptsB.clear();
    m_hullA.clear(); m_hullB.clear();
    m_result.clear();
    m_intersection.clear();

    m_phase = Phase::EditingFirst;

    m_dragIndex = -1;
    m_dragging = false;
    m_pressedOnPoint = false;
    m_moved = false;
    m_suppressClick = false;

    update();
}

QPointF CanvasWidget::toScreen(const Point& p) const {
    const double xmin=-10, xmax=10, ymin=-10, ymax=10;
    double sx = (p.x - xmin)/(xmax-xmin)*width();
    double sy = height() - (p.y - ymin)/(ymax-ymin)*height();
    return {sx, sy};
}
Point CanvasWidget::fromScreen(const QPointF& q) const {
    const double xmin=-10, xmax=10, ymin=-10, ymax=10;
    double x = xmin + q.x()/width()*(xmax-xmin);
    double y = ymin + (height()-q.y())/height()*(ymax-ymin);
    return {x,y};
}

std::optional<int> CanvasWidget::hitPointIndex(const std::vector<Point>& pts,
                                               const QPoint& pos, double tol) const {
    for (int i=0;i<(int)pts.size();++i) {
        QPointF s = toScreen(pts[i]);
        if (QLineF(s, pos).length() <= tol) return i;
    }
    return std::nullopt;
}
std::vector<Point>& CanvasWidget::currentPts() {
    return (m_phase==Phase::EditingSecond) ? m_ptsB : m_ptsA;
}

bool CanvasWidget::nearExistingPoint(const std::vector<Point>& pts,
                                     const Point& p, double tol) const {
    for (const auto& q : pts) {
        double dx = q.x - p.x, dy = q.y - p.y;
        if (dx*dx + dy*dy <= tol*tol) return true;
    }
    return false;
}

void CanvasWidget::recomputeHulls() {
    m_hullA = m_ptsA.empty() ? Polygon{} : PlaneGeometry::convexHull(m_ptsA);
    m_hullB = m_ptsB.empty() ? Polygon{} : PlaneGeometry::convexHull(m_ptsB);
}

void CanvasWidget::recomputeResult() {
    recomputeHulls();
    m_result.clear();
    m_intersection.clear();

    if (m_hullA.empty() && m_hullB.empty()) return;

    switch (m_op) {
    case Op::Intersect: {
        Polygon I = PlaneGeometry::intersectConvex(m_hullA, m_hullB);
        if (!I.empty()) m_result.push_back(std::move(I));
    } break;

    case Op::Difference: {            //  A\B
        const Polygon& Left  = m_diffBA ? m_hullB : m_hullA;
        const Polygon& Right = m_diffBA ? m_hullA : m_hullB;
        Polygon D = PlaneGeometry::differenceConvex(Left, Right);
        if (!D.empty()) m_result.push_back(std::move(D));
        m_intersection = PlaneGeometry::intersectConvex(Left, Right);
    } break;

    case Op::Union: {
        auto parts = PlaneGeometry::unionConvexDecomposed(m_hullA, m_hullB);
        for (auto& p : parts) if (!p.empty()) m_result.push_back(std::move(p));
    } break;
    }
}

void CanvasWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter p(this);

    // Улучшенный фон с градиентом
    QLinearGradient bg(0, 0, 0, height());
    bg.setColorAt(0, QColor(245, 250, 255));
    bg.setColorAt(1, QColor(235, 240, 250));
    p.fillRect(rect(), bg);

    // Светлая сетка для ориентира
    p.setPen(QPen(QColor(220, 230, 240), 1));
    int gridSize = 50;
    for (int x = 0; x < width(); x += gridSize) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        p.drawLine(0, y, width(), y);
    }

    p.setRenderHint(QPainter::Antialiasing, true);

    // Функция для получения пути полигона
    auto pathFromPolyLocal = [&](const Polygon& poly) {
        QPainterPath path;
        if (poly.empty()) return path;
        path.moveTo(toScreen(poly[0]));
        for (size_t i=1;i<poly.size();++i) path.lineTo(toScreen(poly[i]));
        path.closeSubpath();
        return path;
    };

    QPainterPath pathA = pathFromPolyLocal(m_hullA);
    QPainterPath pathB = pathFromPolyLocal(m_hullB);

    // Операция ПЕРЕСЕЧЕНИЕ
    if (m_op == Op::Intersect) {
        // Сначала рисуем оба полигона полупрозрачными
        if (!pathA.isEmpty()) {
            p.setPen(QPen(QColor(30, 144, 255), 2.5));  // Яркий синий
            p.setBrush(QColor(30, 144, 255, 60));       // Полупрозрачный
            p.drawPath(pathA);
        }
        if (!pathB.isEmpty()) {
            p.setPen(QPen(QColor(50, 205, 50), 2.5));   // Яркий зеленый
            p.setBrush(QColor(50, 205, 50, 60));        // Полупрозрачный
            p.drawPath(pathB);
        }

        // Пересечение - ярко-красное с подсветкой
        QPainterPath I = pathA.intersected(pathB);
        if (!I.isEmpty()) {
            p.setPen(QPen(QColor(220, 20, 60), 3.5));   // Толстый красный контур
            p.setBrush(QColor(255, 99, 71, 180));       // Яркая заливка
            p.drawPath(I);
        }
    }
    // Операция ОБЪЕДИНЕНИЕ
    else if (m_op == Op::Union) {
        QPainterPath U = pathA.united(pathB);
        if (!U.isEmpty()) {
            p.setPen(QPen(QColor(138, 43, 226), 3.0));  // Фиолетовый контур
            p.setBrush(QColor(147, 112, 219, 140));     // Фиолетовая заливка
            p.drawPath(U);
        }

        // Пунктирные контуры исходных полигонов
        if (!pathA.isEmpty()) {
            p.setPen(QPen(QColor(30, 144, 255, 150), 1.5, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawPath(pathA);
        }
        if (!pathB.isEmpty()) {
            p.setPen(QPen(QColor(50, 205, 50, 150), 1.5, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawPath(pathB);
        }
    }
    // Операция РАЗНОСТЬ - ПРАВИЛЬНО!
    else if (m_op == Op::Difference) {
        // 1. Полигон A - синий
        if (!pathA.isEmpty()) {
            p.setPen(QPen(QColor(30, 144, 255), 2.5));  // Синий контур
            p.setBrush(QColor(65, 105, 225, 100));      // RoyalBlue заливка
            p.drawPath(pathA);
        }

        // 2. Полигон B - зеленый (заливаем)
        if (!pathB.isEmpty()) {
            p.setPen(QPen(QColor(50, 205, 50, 180), 2.0)); // Зеленый контур
            p.setBrush(QColor(50, 205, 50, 80));           // Зеленая заливка
            p.drawPath(pathB);
        }

        // 3. Область пересечения (A ∩ B) - ЗАКРАШИВАЕМ БЕЛЫМ (вырезаем)
        QPainterPath I = pathA.intersected(pathB);
        if (!I.isEmpty()) {
            p.setPen(QPen(Qt::white, 2));                // Белый контур
            p.setBrush(Qt::white);                       // Белая заливка
            p.drawPath(I);
        }
    }

    // Выпуклые оболочки - пунктирные линии поверх всего
    auto drawHull = [&](const Polygon& hull, QColor color) {
        if (hull.empty()) return;
        QPen pen(color, 2.0);
        pen.setStyle(Qt::DashLine);
        pen.setDashPattern({5, 3});
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        QPainterPath ph = pathFromPolyLocal(hull);
        p.drawPath(ph);
    };

    if (m_op != Op::Difference || m_phase != Phase::Ready) {
        drawHull(m_hullA, QColor(0, 0, 139));  // Темно-синий
        drawHull(m_hullB, QColor(0, 100, 0));  // Темно-зеленый
    }

    // Точки - яркие и с обводкой
    auto drawPoints = [&](const std::vector<Point>& pts, QColor c) {
        p.setPen(QPen(c.darker(), 1.5));
        p.setBrush(c);
        for (const auto& v : pts) {
            QPointF s = toScreen(v);
            p.drawEllipse(QRectF(s.x()-4, s.y()-4, 8, 8));
        }
    };

    drawPoints(m_ptsA, QColor(30, 144, 255));  // Синие точки
    drawPoints(m_ptsB, QColor(50, 205, 50));   // Зеленые точки

    // Статусная строка вверху (исправленная - без двойного текста)
    QFont font = p.font();
    font.setPointSize(11);
    font.setBold(true);
    p.setFont(font);

    QString status;
    QColor statusColor;

    switch (m_phase) {
    case Phase::EditingFirst:
        status = "РЕЖИМ: Добавление точек полигона A";
        statusColor = QColor(30, 144, 255);
        break;
    case Phase::EditingSecond:
        status = "РЕЖИМ: Добавление точек полигона B";
        statusColor = QColor(50, 205, 50);
        break;
    case Phase::Ready:
        status = "РЕЖИМ: Оба полигона готовы";
        statusColor = QColor(138, 43, 226);
        break;
    }

    // Фон для статуса
    p.setBrush(QColor(255, 255, 255, 230));
    p.setPen(QPen(QColor(200, 210, 220), 1));
    p.drawRect(10, 10, 450, 35);

    // Текст статуса (только один раз!)
    p.setPen(statusColor);
    p.drawText(20, 32, status);

    // Информация о количестве точек
    p.setPen(QColor(100, 100, 120));
    font.setPointSize(10);
    font.setBold(false);
    p.setFont(font);
    p.drawText(width() - 220, 32,
               QString("A: %1 точек | B: %2 точек").arg(m_ptsA.size()).arg(m_ptsB.size()));
}

void CanvasWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;

    m_pressPos = e->pos();
    m_moved = false;

    auto& pts = currentPts();
    if (auto idx = hitPointIndex(pts, e->pos()); idx.has_value()) {
        m_dragIndex = *idx;
        m_dragging = true;
        m_pressedOnPoint = true;
    } else {
        m_dragIndex = -1;
        m_dragging = false;
        m_pressedOnPoint = false;
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* e) {
    if ((e->pos() - m_pressPos).manhattanLength() > 3) m_moved = true;

    if (m_dragging && m_dragIndex >= 0) {
        auto& pts = currentPts();
        if (m_dragIndex < (int)pts.size()) {
            pts[m_dragIndex] = fromScreen(e->position());
            recomputeResult();
            update();
        }
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;

    // Если был drag существующей точки — ничего не добавляем
    if (m_dragging) {
        m_dragging = false;
        m_dragIndex = -1;
        m_pressedOnPoint = false;
        m_moved = false;
        return;
    }

    // если только что был double click — подавляем добавление
    if (m_suppressClick) {
        m_suppressClick = false;
    } else if (!m_pressedOnPoint && !m_moved) {
        // клик по пустому месту — добавляем одну точку
        auto& pts = currentPts();
        Point w = fromScreen(e->position());
        if (!nearExistingPoint(pts, w, 0.15)) {
            pts.push_back(w);
            recomputeResult();
            update();
        }
    }

    m_dragIndex = -1;
    m_pressedOnPoint = false;
    m_moved = false;
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent* e) {
    Q_UNUSED(e);

    // подавляем последующий release
    m_suppressClick = true;

    if (m_phase == Phase::EditingFirst) finalizeFirst();
    else if (m_phase == Phase::EditingSecond) finalizeSecond();
}
