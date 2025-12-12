#include "CanvasWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPen>
#include <QStyleOption>

using PlaneGeometry::Point;
using PlaneGeometry::Polygon;

CanvasWidget::CanvasWidget(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(true);

    // Установим минимальный размер
    setMinimumSize(800, 600);
}

void CanvasWidget::setOp(Op op) {
    m_op = op;
    update();
}

void CanvasWidget::finalizeFirst() {
    if (m_polyA.size() >= 3) {
        m_closedA = true;
        m_phase   = Phase::EditingSecond;
        update();
    }
}

void CanvasWidget::finalizeSecond() {
    if (m_polyB.size() >= 3) {
        m_closedB = true;
        m_phase   = Phase::Ready;
        update();
    }
}

void CanvasWidget::clearAll() {
    m_polyA.clear();
    m_polyB.clear();
    m_closedA   = false;
    m_closedB   = false;
    m_dragging  = false;
    m_dragInA   = true;
    m_dragIndex = -1;
    m_phase     = Phase::EditingFirst;
    update();
}

void CanvasWidget::addPointForCurrent(const QPointF& pos) {
    Point p{pos.x(), pos.y()};
    if (m_phase == Phase::EditingFirst) {
        m_polyA.push_back(p);
    } else if (m_phase == Phase::EditingSecond) {
        m_polyB.push_back(p);
    }
}

bool CanvasWidget::pickVertex(const QPointF& pos) {
    auto tryPick = [&](Polygon& poly, bool inA) -> bool {
        for (int i = 0; i < (int)poly.size(); ++i) {
            QPointF v(poly[i].x, poly[i].y);
            if (QLineF(v, pos).length() <= m_hitRadiusPx) {
                m_dragging  = true;
                m_dragInA   = inA;
                m_dragIndex = i;
                return true;
            }
        }
        return false;
    };

    if (tryPick(m_polyA, true))  return true;
    if (tryPick(m_polyB, false)) return true;
    return false;
}

QPainterPath CanvasWidget::pathFromPoly(const Polygon& poly, bool closed) const {
    QPainterPath path;
    if (poly.empty()) return path;

    path.moveTo(poly[0].x, poly[0].y);
    for (std::size_t i = 1; i < poly.size(); ++i)
        path.lineTo(poly[i].x, poly[i].y);

    if (closed)
        path.closeSubpath();

    path.setFillRule(Qt::WindingFill);
    return path;
}

void CanvasWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // 1. Рисуем фон в клетку серо-голубого цвета на ВСЕЙ области
    p.fillRect(rect(), QColor(240, 245, 250)); // Светлый серо-голубой фон

    // Рисуем сетку (клетку) - увеличенный размер
    p.setPen(QPen(QColor(210, 220, 230), 1)); // Серо-голубые линии
    const int gridSize = 30; // Увеличенный размер клетки
    for (int x = 0; x < width(); x += gridSize) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize) {
        p.drawLine(0, y, width(), y);
    }

    // Более темные линии каждые 5 клеток
    p.setPen(QPen(QColor(190, 200, 215), 1.5));
    for (int x = 0; x < width(); x += gridSize * 5) {
        p.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += gridSize * 5) {
        p.drawLine(0, y, width(), y);
    }

    // 2. Рисуем полигоны
    auto drawPoly = [&](const Polygon& poly, bool closed,
                        const QColor& edgeColor, const QColor& fillColor, const QString& label) {
        if (poly.empty()) return;

        // Рисуем заливку полигона
        if (closed && poly.size() >= 3) {
            QPainterPath path = pathFromPoly(poly, closed);
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(fillColor));
            p.drawPath(path);
        }

        // Рисуем контур
        QPen pen(edgeColor, 3);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        QPainterPath path = pathFromPoly(poly, closed);
        p.drawPath(path);

        // Рисуем вершины
        p.setBrush(QBrush(edgeColor));
        p.setPen(QPen(Qt::white, 2));
        const double r = 7.0;
        for (const auto& pt : poly) {
            p.drawEllipse(QPointF(pt.x, pt.y), r, r);
        }

        // Подписываем полигон
        if (!poly.empty() && !label.isEmpty()) {
            p.setPen(edgeColor);
            p.setFont(QFont("Arial", 11, QFont::Bold));
            p.drawText(QPointF(poly[0].x + 20, poly[0].y - 15), label);
        }

        // Если полигон не замкнут, рисуем пунктирную линию к началу
        if (!closed && poly.size() > 2) {
            QPen dashPen(edgeColor, 2, Qt::DashLine);
            p.setPen(dashPen);
            p.drawLine(QPointF(poly[0].x, poly[0].y),
                       QPointF(poly.back().x, poly.back().y));
        }
    };

    // Полигон A - ОРАНЖЕВЫЙ
    QColor polyAColor(255, 165, 0); // Оранжевый
    QColor polyAFill(255, 165, 0, 100); // Полупрозрачный оранжевый
    drawPoly(m_polyA, m_closedA, polyAColor, polyAFill, "A");

    // Полигон B - СИНИЙ
    QColor polyBColor(0, 120, 215); // Синий
    QColor polyBFill(0, 120, 215, 100); // Полупрозрачный синий
    drawPoly(m_polyB, m_closedB, polyBColor, polyBFill, "B");

    // 3. Выполняем булевы операции только если оба полигона замкнуты
    if (m_closedA && m_closedB) {
        QPainterPath pathA = pathFromPoly(m_polyA, true);
        QPainterPath pathB = pathFromPoly(m_polyB, true);

        QPainterPath resultPath;
        QColor resultColor;
        QString operationText;

        if (m_op == Op::Intersect) {
            resultPath = pathA.intersected(pathB);
            resultColor = QColor(255, 100, 100, 200); // Красный для пересечения
            operationText = "Пересечение";

            // Рисуем результат пересечения
            if (!resultPath.isEmpty()) {
                p.setPen(QPen(resultColor.darker(), 4));
                p.setBrush(QBrush(resultColor));
                p.drawPath(resultPath);
            }
        } else if (m_op == Op::Union) {
            resultPath = pathA.united(pathB);
            resultColor = QColor(100, 200, 100, 200); // Зеленый для объединения
            operationText = "Объединение";

            // Рисуем результат объединения
            if (!resultPath.isEmpty()) {
                p.setPen(QPen(resultColor.darker(), 4));
                p.setBrush(QBrush(resultColor));
                p.drawPath(resultPath);
            }
        } else if (m_op == Op::Difference) {
            // Для разности (A - B) вырезаем часть B из A белым цветом
            resultPath = pathA.subtracted(pathB);
            operationText = "Разность (A - B)";

            // Сначала рисуем полигон A полностью
            p.setPen(QPen(polyAColor, 3));
            p.setBrush(QBrush(polyAFill));
            p.drawPath(pathA);

            // Находим пересечение A и B
            QPainterPath intersectPath = pathA.intersected(pathB);

            // Пересечение рисуем БЕЛЫМ цветом (вырезаем)
            if (!intersectPath.isEmpty()) {
                p.setPen(QPen(Qt::white, 3));
                p.setBrush(QBrush(Qt::white));
                p.drawPath(intersectPath);

                // Контур вырезанной области - пунктир
                QPen dashPen(QColor(150, 150, 150), 2, Qt::DashLine);
                p.setPen(dashPen);
                p.setBrush(Qt::NoBrush);
                p.drawPath(intersectPath);
            }
        }

        // Подписываем результат (для всех операций кроме разности)
        if (m_op != Op::Difference && !resultPath.isEmpty() && !resultPath.boundingRect().isEmpty()) {
            QRectF bounds = resultPath.boundingRect();
            p.setPen(Qt::black);
            p.setFont(QFont("Arial", 12, QFont::Bold));

            // Фон для текста операции
            QFont operationFont("Arial", 12, QFont::Bold);
            QFontMetrics operationFm(operationFont);
            int operationWidth = operationFm.horizontalAdvance(operationText);
            QRect operationRect(bounds.center().x() - operationWidth/2 - 10,
                                bounds.center().y() - 20,
                                operationWidth + 20, 25);

            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 255, 255, 220));
            p.drawRoundedRect(operationRect, 5, 5);

            p.setPen(resultColor.darker());
            p.drawText(operationRect, Qt::AlignCenter, operationText);
        }

        // Для разности подписываем отдельно
        if (m_op == Op::Difference) {
            // Находим подходящее место для надписи (центр полигона A)
            if (!pathA.boundingRect().isEmpty()) {
                QRectF bounds = pathA.boundingRect();
                p.setPen(Qt::black);
                p.setFont(QFont("Arial", 12, QFont::Bold));

                QFont operationFont("Arial", 12, QFont::Bold);
                QFontMetrics operationFm(operationFont);
                int operationWidth = operationFm.horizontalAdvance(operationText);
                QRect operationRect(bounds.center().x() - operationWidth/2 - 10,
                                    bounds.center().y() - 20,
                                    operationWidth + 20, 25);

                p.setPen(Qt::NoPen);
                p.setBrush(QColor(255, 255, 255, 220));
                p.drawRoundedRect(operationRect, 5, 5);

                p.setPen(polyAColor.darker());
                p.drawText(operationRect, Qt::AlignCenter, operationText);
            }
        }
    }

    // 4. Рисуем легенду в правом нижнем углу
    const int legendWidth = 200;
    const int legendHeight = 130;
    QRect legendRect(width() - legendWidth - 15, height() - legendHeight - 15,
                     legendWidth, legendHeight);

    // Фон легенды с закругленными углами и тенью
    p.setPen(QPen(QColor(100, 100, 120), 1));
    p.setBrush(QBrush(QColor(255, 255, 255, 240)));
    p.drawRoundedRect(legendRect, 10, 10);

    p.setFont(QFont("Arial", 10, QFont::Bold));
    p.setPen(QColor(60, 60, 80));

    // Заголовок легенды - по центру
    QString legendTitle = "Легенда";
    QFontMetrics legendFm(p.font());
    int legendTitleWidth = legendFm.horizontalAdvance(legendTitle);
    p.drawText(legendRect.x() + (legendWidth - legendTitleWidth) / 2, legendRect.y() + 25, legendTitle);

    // Разделительная линия под заголовком
    p.setPen(QPen(QColor(200, 200, 210), 1));
    p.drawLine(legendRect.x() + 15, legendRect.y() + 30, legendRect.right() - 15, legendRect.y() + 30);

    // Полигон A
    p.setBrush(polyAFill);
    p.setPen(QPen(polyAColor, 2));
    p.drawRect(legendRect.x() + 20, legendRect.y() + 40, 16, 16);
    p.setPen(QColor(60, 60, 80));
    p.drawText(legendRect.x() + 45, legendRect.y() + 53, "Полигон A");

    // Полигон B
    p.setBrush(polyBFill);
    p.setPen(QPen(polyBColor, 2));
    p.drawRect(legendRect.x() + 20, legendRect.y() + 65, 16, 16);
    p.setPen(QColor(60, 60, 80));
    p.drawText(legendRect.x() + 45, legendRect.y() + 78, "Полигон B");

    // Разделительная линия
    p.setPen(QPen(QColor(200, 200, 210), 1));
    p.drawLine(legendRect.x() + 15, legendRect.y() + 85, legendRect.right() - 15, legendRect.y() + 85);

    // Инструкция
    p.setFont(QFont("Arial", 9));
    p.setPen(QColor(100, 100, 120));
    p.drawText(legendRect.x() + 20, legendRect.y() + 100, "ЛКМ - добавить точку");
    p.drawText(legendRect.x() + 20, legendRect.y() + 115, "Перетащите точку - переместить");
}

void CanvasWidget::mousePressEvent(QMouseEvent* event) {
    const QPointF pos = event->position();

    if (event->button() == Qt::LeftButton) {
        // Сначала пробуем схватить вершину
        if (!pickVertex(pos)) {
            // Если не попали в вершину — добавляем новую
            addPointForCurrent(pos);
        }
        update();
    }

    QWidget::mousePressEvent(event);
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && m_dragIndex >= 0) {
        QPointF pos = event->position();
        Point p{pos.x(), pos.y()};

        if (m_dragInA) {
            if (m_dragIndex >= 0 && m_dragIndex < (int)m_polyA.size())
                m_polyA[m_dragIndex] = p;
        } else {
            if (m_dragIndex >= 0 && m_dragIndex < (int)m_polyB.size())
                m_polyB[m_dragIndex] = p;
        }
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging  = false;
        m_dragIndex = -1;
    }
    QWidget::mouseReleaseEvent(event);
}
