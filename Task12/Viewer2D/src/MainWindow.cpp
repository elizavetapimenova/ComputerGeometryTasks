#include "MainWindow.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <algorithm>
#include <cmath>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Произвольный полигон с дырками и самопересечениями");
    resize(1000, 700);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Панель кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    buildPolygonBtn = new QPushButton("Зафиксировать полигон", this);
    addHoleBtn = new QPushButton("Добавить дырку", this);
    convexHullBtn = new QPushButton("Построить выпуклую оболочку", this);
    clearBtn = new QPushButton("Очистить всё", this);

    buttonLayout->addWidget(buildPolygonBtn);
    buttonLayout->addWidget(addHoleBtn);
    buttonLayout->addWidget(convexHullBtn);
    buttonLayout->addWidget(clearBtn);
    buttonLayout->addStretch();

    // Статус
    statusLabel = new QLabel("Кликайте ЛКМ для создания основного полигона (минимум 3 точки)", this);
    statusLabel->setStyleSheet("QLabel { padding: 10px; background: #f0f0f0; border: 2px solid #ccc; font-size: 12pt; }");
    statusLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();

    // Соединения
    connect(buildPolygonBtn, &QPushButton::clicked, this, &MainWindow::buildPolygon);
    connect(addHoleBtn, &QPushButton::clicked, this, &MainWindow::addHole);
    connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearAll);
    connect(convexHullBtn, &QPushButton::clicked, this, &MainWindow::buildConvexHull);

    // Статусбар
    statusBar()->showMessage("ЛКМ - добавить точку | Перетаскивайте тестовые точки | Двойной клик - завершить");
}

void MainWindow::buildPolygon() {
    if (currentContour.size() >= 3) {
        if (!creatingHole) {
            // Фиксируем основной полигон
            polygons.clear();
            polygons.append(currentContour);
            polygonBuilt = true;
            currentContour.clear();

            statusLabel->setText("Основной полигон построен! Можно добавить дырку или тестовые точки");
            statusLabel->setStyleSheet("QLabel { padding: 10px; background: #d4edda; border: 2px solid #c3e6cb; font-size: 12pt; }");

            rebuildDelta();
        } else {
            // Фиксируем дырку
            if (currentContour.size() >= 3) {
                polygons.append(currentContour);
                currentContour.clear();
                creatingHole = false;

                statusLabel->setText(QString("Дырка добавлена! Всего дырок: %1").arg(polygons.size() - 1));
                statusLabel->setStyleSheet("QLabel { padding: 10px; background: #fff3cd; border: 2px solid #ffeeba; font-size: 12pt; }");

                rebuildDelta();
            } else {
                QMessageBox::warning(this, "Ошибка", "Дырка должна содержать минимум 3 точки!");
                return;
            }
        }
        update();
    } else {
        QMessageBox::warning(this, "Ошибка", "Нужно минимум 3 точки!");
    }
}

void MainWindow::addHole() {
    if (!polygonBuilt) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте основной полигон!");
        return;
    }

    creatingHole = true;
    currentContour.clear();

    statusLabel->setText("Создание дырки. Кликайте ЛКМ для добавления точек дырки");
    statusLabel->setStyleSheet("QLabel { padding: 10px; background: #e2f0fd; border: 2px solid #b8daff; font-size: 12pt; }");
    statusBar()->showMessage("Создание дырки - добавляйте точки внутри основного полигона");

    update();
}

void MainWindow::clearAll() {
    currentContour.clear();
    polygons.clear();
    testPoints.clear();
    convexHullPoints.clear();
    polygonBuilt = false;
    creatingHole = false;
    draggedPointIndex = -1;
    dragging = false;
    delta = 5.0;

    statusLabel->setText("Кликайте ЛКМ для создания основного полигона (минимум 3 точки)");
    statusLabel->setStyleSheet("QLabel { padding: 10px; background: #f0f0f0; border: 2px solid #ccc; font-size: 12pt; }");
    statusBar()->showMessage("Очищено. Начните заново.");

    update();
}

void MainWindow::buildConvexHull() {
    if (polygons.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала создайте полигон!");
        return;
    }

    // Собираем все точки
    vector<Point> allPoints;
    for (const auto& poly : polygons) {
        for (const auto& p : poly) {
            allPoints.push_back(p);
        }
    }

    // Строим выпуклую оболочку
    convexHullPoints.clear();
    auto hull = convexHull(allPoints);
    for (const auto& p : hull) {
        convexHullPoints.append(p);
    }

    statusLabel->setText(QString("Выпуклая оболочка построена: %1 вершин").arg(convexHullPoints.size()));
    statusBar()->showMessage("Выпуклая оболочка построена");

    update();
}

void MainWindow::rebuildDelta() {
    if (polygons.isEmpty()) return;

    vector<Point> allPoints;
    for (const auto& poly : polygons) {
        for (const auto& p : poly) {
            allPoints.push_back(p);
        }
    }

    delta = minDistance(allPoints) / 10.0;
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), QColor(250, 250, 250));

    // Рисуем выпуклую оболочку
    if (!convexHullPoints.isEmpty()) {
        painter.setPen(QPen(Qt::green, 3, Qt::DashDotLine));
        painter.setBrush(Qt::NoBrush);

        QPolygonF hullPoly;
        for (const auto& p : convexHullPoints) {
            hullPoly << QPointF(p.x, p.y);
        }
        painter.drawPolygon(hullPoly);
    }

    // Рисуем все полигоны (основной + дырки)
    for (int polyIndex = 0; polyIndex < polygons.size(); ++polyIndex) {
        const auto& poly = polygons[polyIndex];
        if (poly.size() < 2) continue;

        // Разный стиль для основного полигона и дырок
        if (polyIndex == 0) {
            // Основной полигон
            painter.setPen(QPen(Qt::blue, 2));
            painter.setBrush(QColor(200, 230, 255, 100)); // Полупрозрачная заливка
        } else {
            // Дырки
            painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
            painter.setBrush(Qt::white); // Белый цвет для вырезания
        }

        QPolygonF qpoly;
        for (const auto& p : poly) {
            qpoly << QPointF(p.x, p.y);
        }
        painter.drawPolygon(qpoly);

        // Точки полигона
        painter.setBrush(polyIndex == 0 ? Qt::blue : Qt::magenta);
        painter.setPen(Qt::black);
        for (const auto& p : poly) {
            painter.drawEllipse(QPointF(p.x, p.y), 6, 6);
        }
    }

    // Рисуем текущий контур (если создаем)
    if (!currentContour.isEmpty()) {
        painter.setPen(QPen(creatingHole ? Qt::magenta : Qt::blue, 2, Qt::DashLine));
        painter.setBrush(Qt::NoBrush);

        for (int i = 1; i < currentContour.size(); ++i) {
            painter.drawLine(QPointF(currentContour[i-1].x, currentContour[i-1].y),
                             QPointF(currentContour[i].x, currentContour[i].y));
        }

        // Точки текущего контура
        painter.setBrush(creatingHole ? Qt::magenta : Qt::blue);
        for (const auto& p : currentContour) {
            painter.drawEllipse(QPointF(p.x, p.y), 6, 6);
        }
    }

    // Рисуем тестовые точки
    for (int i = 0; i < testPoints.size(); ++i) {
        const auto& p = testPoints[i];
        QColor color = Qt::red;
        QString status;

        if (polygonBuilt && !polygons.isEmpty()) {
            // Конвертируем в формат для Geometry
            vector<vector<Point>> stdPolygons;
            for (const auto& poly : polygons) {
                stdPolygons.push_back(vector<Point>(poly.begin(), poly.end()));
            }

            PointPosition pos = pointInPolygon(p, stdPolygons, delta);
            color = getColorForPosition(pos);
            status = getStatusText(pos);

            // Для последней точки показываем статус
            if (i == testPoints.size() - 1) {
                painter.setPen(Qt::black);
                painter.drawText(QPointF(p.x + 15, p.y - 15), status);
            }
        }

        painter.setBrush(color);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(QPointF(p.x, p.y), 8, 8);

        // Номер точки
        painter.setPen(Qt::black);
        painter.drawText(QPointF(p.x - 5, p.y - 10), QString("P%1").arg(i+1));
    }

    // Легенда и информация
    painter.setPen(Qt::black);

    // Информация
    int totalHoles = polygons.size() > 0 ? polygons.size() - 1 : 0;
    painter.drawText(20, 30, QString("Основной полигон: %1 точек").arg(polygons.size() > 0 ? polygons[0].size() : 0));
    painter.drawText(20, 50, QString("Дырок: %1").arg(totalHoles));
    painter.drawText(20, 70, QString("Тестовых точек: %1").arg(testPoints.size()));
    if (polygonBuilt) {
        painter.drawText(20, 90, QString("Delta: %1").arg(delta, 0, 'f', 2));
    }
    if (!convexHullPoints.isEmpty()) {
        painter.drawText(20, 110, QString("Выпуклая оболочка: %1 вершин").arg(convexHullPoints.size()));
    }

    // Легенда внизу
    int y = height() - 120;
    painter.drawText(20, y, "Легенда:");

    painter.setBrush(Qt::blue);
    painter.drawEllipse(20, y + 20, 8, 8);
    painter.drawText(40, y + 27, "- основной полигон");

    painter.setBrush(Qt::magenta);
    painter.drawEllipse(20, y + 40, 8, 8);
    painter.drawText(40, y + 47, "- дырки");

    painter.setBrush(QColor(0, 255, 0));
    painter.drawEllipse(20, y + 60, 8, 8);
    painter.drawText(40, y + 67, "- точка внутри");

    painter.setBrush(Qt::red);
    painter.drawEllipse(20, y + 80, 8, 8);
    painter.drawText(40, y + 87, "- точка снаружи");

    painter.setBrush(QColor(255, 255, 0));
    painter.drawEllipse(200, y + 20, 8, 8);
    painter.drawText(220, y + 27, "- близко к границе");

    painter.setBrush(Qt::blue);
    painter.drawEllipse(200, y + 40, 8, 8);
    painter.drawText(220, y + 47, "- на границе");

    painter.setBrush(Qt::green);
    painter.setPen(QPen(Qt::green, 1, Qt::DashDotLine));
    painter.drawEllipse(200, y + 60, 8, 8);
    painter.drawText(220, y + 67, "- выпуклая оболочка");
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPointF pos = event->pos();

    // Проверяем клик на тестовых точках для перетаскивания
    if (polygonBuilt) {
        for (int i = 0; i < testPoints.size(); ++i) {
            double dx = testPoints[i].x - pos.x();
            double dy = testPoints[i].y - pos.y();
            if (dx*dx + dy*dy < 100) { // радиус 10 пикселей
                draggedPointIndex = i;
                dragging = true;
                update();
                return;
            }
        }
    }

    // Добавление точки в текущий контур или как тестовую точку
    if (event->button() == Qt::LeftButton) {
        if (!polygonBuilt || creatingHole) {
            // Добавляем точку в текущий контур (основной полигон или дырку)
            currentContour.append(Point(pos.x(), pos.y()));

            if (creatingHole) {
                statusBar()->showMessage(QString("Точка дырки: %1").arg(currentContour.size()));
            } else {
                statusBar()->showMessage(QString("Точка полигона: %1").arg(currentContour.size()));
            }

            update();
        } else if (polygonBuilt) {
            // Добавляем тестовую точку
            testPoints.append(Point(pos.x(), pos.y()));
            updateTestPointStatus();
            update();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (dragging && draggedPointIndex >= 0 && draggedPointIndex < testPoints.size()) {
        testPoints[draggedPointIndex] = Point(event->pos().x(), event->pos().y());
        updateTestPointStatus();
        update();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    dragging = false;
    draggedPointIndex = -1;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if (!polygonBuilt || creatingHole) {
        // Двойной клик завершает текущий контур
        buildPolygon();
    }
}

QColor MainWindow::getColorForPosition(PointPosition pos) {
    switch (pos) {
    case PointPosition::Inside: return Qt::green;
    case PointPosition::Outside: return Qt::red;
    case PointPosition::OnBoundary: return Qt::blue;
    case PointPosition::NearBoundary: return QColor(255, 255, 0); // желтый
    }
    return Qt::black;
}

QString MainWindow::getStatusText(PointPosition pos) {
    switch (pos) {
    case PointPosition::Inside: return "ВНУТРИ";
    case PointPosition::Outside: return "СНАРУЖИ";
    case PointPosition::OnBoundary: return "НА ГРАНИЦЕ";
    case PointPosition::NearBoundary: return QString("БЛИЗКО К ГРАНИЦЕ (δ=%1)").arg(delta, 0, 'f', 2);
    }
    return "";
}

void MainWindow::updateTestPointStatus() {
    if (!polygonBuilt || polygons.isEmpty() || testPoints.isEmpty()) return;

    // Конвертируем в формат для Geometry
    vector<vector<Point>> stdPolygons;
    for (const auto& poly : polygons) {
        stdPolygons.push_back(vector<Point>(poly.begin(), poly.end()));
    }

    const Point& lastPoint = testPoints.last();
    PointPosition pos = pointInPolygon(lastPoint, stdPolygons, delta);

    QString status = QString("Тестовая точка P%1: %2")
                         .arg(testPoints.size())
                         .arg(getStatusText(pos));

    statusLabel->setText(status);

    // Меняем цвет в зависимости от положения
    QString style;
    switch (pos) {
    case PointPosition::Inside:
        style = "background: #d4edda; border-color: #c3e6cb; color: #155724;";
        break;
    case PointPosition::Outside:
        style = "background: #f8d7da; border-color: #f5c6cb; color: #721c24;";
        break;
    case PointPosition::OnBoundary:
        style = "background: #cce5ff; border-color: #b8daff; color: #004085;";
        break;
    case PointPosition::NearBoundary:
        style = "background: #fff3cd; border-color: #ffeeba; color: #856404;";
        break;
    }
    statusLabel->setStyleSheet(QString("QLabel { padding: 10px; %1 font-size: 12pt; }").arg(style));

    statusBar()->showMessage(getStatusText(pos));
}
