#include "MainWindow.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMessageBox>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace std;

// Геометрические функции (перенесены сюда)
vector<Point> MainWindow::convexHull(vector<Point> points) {
    if(points.size() <= 3) return points;
    sort(points.begin(), points.end(), [](const Point &a, const Point &b){
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    vector<Point> hull;
    // Нижняя
    for(auto &p: points) {
        while(hull.size() >= 2 && ((hull.back()-hull[hull.size()-2]).cross(p-hull.back())) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    // Верхняя
    size_t t = hull.size() + 1;
    for(int i = points.size()-2; i>=0; --i){
        auto &p = points[i];
        while(hull.size() >= t && ((hull.back()-hull[hull.size()-2]).cross(p-hull.back())) <=0)
            hull.pop_back();
        hull.push_back(p);
    }
    hull.pop_back();
    return hull;
}

double MainWindow::minDistance(const vector<Point> &polygon){
    double minDist = numeric_limits<double>::max();
    for(size_t i=0;i<polygon.size();++i){
        for(size_t j=i+1;j<polygon.size();++j){
            double d = sqrt(polygon[i].dist2(polygon[j]));
            if(d < minDist) minDist = d;
        }
    }
    return minDist;
}

PointPosition MainWindow::pointInPolygon(const Point &p, const vector<Point> &polygon, double delta){
    bool inside = false;
    size_t n = polygon.size();

    for(size_t i=0,j=n-1;i<n;j=i++){
        Point pi = polygon[i], pj = polygon[j];

        // Проверка на точное совпадение с вершиной
        if (fabs(p.x - pi.x) < 1e-9 && fabs(p.y - pi.y) < 1e-9) {
            return PointPosition::OnBoundary;
        }

        // Проверка на нахождение на ребре
        double crossProduct = (p.x - pi.x)*(pj.y - pi.y) - (p.y - pi.y)*(pj.x - pi.x);
        if (fabs(crossProduct) < 1e-9) {
            if (p.x >= min(pi.x, pj.x) && p.x <= max(pi.x, pj.x) &&
                p.y >= min(pi.y, pj.y) && p.y <= max(pi.y, pj.y)) {
                return PointPosition::OnBoundary;
            }
        }

        if( ((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y)/(pj.y - pi.y) + pi.x) )
            inside = !inside;

        // Проверка близости к границе
        double dx = pj.x - pi.x, dy = pj.y - pi.y;
        double t = ((p.x - pi.x)*dx + (p.y - pi.y)*dy)/(dx*dx + dy*dy);
        if(t>=0 && t<=1){
            double px = pi.x + t*dx;
            double py = pi.y + t*dy;
            double d2 = (p.x - px)*(p.x - px) + (p.y - py)*(p.y - py);
            if(d2 < delta*delta) return PointPosition::NearBoundary;
        }
    }

    return inside ? PointPosition::Inside : PointPosition::Outside;
}

// Конструктор с улучшенным интерфейсом
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Проверка точки в выпуклой оболочке");
    resize(1000, 700);

    hullBuilt = false;
    delta = 5.0;
    draggedIndex = -1;
    draggingPolygonPoint = false;

    // Центральный виджет
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Панель кнопок
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buildButton = new QPushButton("Построить выпуклую оболочку", this);
    clearButton = new QPushButton("Очистить всё", this);

    buttonLayout->addWidget(buildButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();

    // Статус бар
    statusLabel = new QLabel("Кликайте ЛКМ для добавления точек полигона. Нужно минимум 3 точки.", this);
    statusLabel->setStyleSheet("QLabel { padding: 5px; background: #f0f0f0; border: 1px solid #ccc; }");
    statusLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addStretch();

    // Подсказка в статусбаре
    statusBar()->showMessage("Двойной клик = построить оболочку | Перетаскивайте точки мышкой");

    // Соединения
    connect(buildButton, &QPushButton::clicked, this, &MainWindow::buildConvexHull);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearAll);
}

void MainWindow::buildConvexHull(){
    if(polygonPoints.size()>=3){
        hullBuilt = true;
        rebuildHull();
        rebuildDelta();
        statusLabel->setText(QString("Выпуклая оболочка построена! Добавьте тестовую точку. Delta = %1")
                                 .arg(delta, 0, 'f', 2));
        statusLabel->setStyleSheet("QLabel { padding: 5px; background: #d4edda; border: 1px solid #c3e6cb; color: #155724; }");
        update();
    } else {
        QMessageBox::warning(this, "Недостаточно точек",
                             "Для построения выпуклой оболочки нужно минимум 3 точки!");
    }
}

void MainWindow::clearAll(){
    polygonPoints.clear();
    hull.clear();
    extraPoints.clear();
    hullBuilt = false;
    delta = 5.0;
    draggedIndex = -1;

    statusLabel->setText("Кликайте ЛКМ для добавления точек полигона. Нужно минимум 3 точки.");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background: #f0f0f0; border: 1px solid #ccc; }");
    statusBar()->showMessage("Очищено. Начните заново.");
    update();
}

void MainWindow::rebuildHull(){
    std::vector<Point> pts(polygonPoints.begin(), polygonPoints.end());
    std::vector<Point> h = convexHull(pts);
    hull.clear();
    for(const auto &p: h) hull.append(p);
}

void MainWindow::rebuildDelta(){
    std::vector<Point> h(hull.begin(), hull.end());
    delta = minDistance(h)/10.0;
}

QColor MainWindow::getColorForPosition(PointPosition position){
    switch(position){
    case PointPosition::Inside: return QColor(0, 255, 0);      // зеленый
    case PointPosition::Outside: return QColor(255, 0, 0);     // красный
    case PointPosition::OnBoundary: return QColor(0, 0, 255);  // синий
    case PointPosition::NearBoundary: return QColor(255, 255, 0); // желтый
    default: return Qt::black;
    }
}

QString MainWindow::getStatusText(PointPosition position){
    switch(position){
    case PointPosition::Inside: return "Точка ВНУТРИ полигона";
    case PointPosition::Outside: return "Точка СНАРУЖИ полигона";
    case PointPosition::OnBoundary: return "Точка НА ГРАНИЦЕ полигона";
    case PointPosition::NearBoundary: return QString("Точка БЛИЗКО К ГРАНИЦЕ (delta = %1)").arg(delta, 0, 'f', 2);
    default: return "Неизвестно";
    }
}

void MainWindow::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), QColor(250, 250, 250));

    // Рисуем выпуклую оболочку с заливкой
    if(hullBuilt && hull.size()>=2){
        QPolygonF hullPoly;
        for(const auto &p: hull){
            hullPoly << QPointF(p.x, p.y);
        }

        // Заливка оболочки
        painter.setBrush(QColor(200, 255, 200, 100)); // полупрозрачная зеленая
        painter.setPen(QPen(QColor(0, 150, 0), 3)); // толстая зеленая граница
        painter.drawPolygon(hullPoly);

        // Текст с количеством вершин
        painter.setPen(Qt::darkGreen);
        painter.drawText(20, 40, QString("Вершин оболочки: %1").arg(hull.size()));
    }

    // Рисуем исходный полигон (тонкие линии)
    if(polygonPoints.size()>=2 && !hullBuilt){
        painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
        for(int i=0;i<polygonPoints.size()-1;++i){
            painter.drawLine(polygonPoints[i].x, polygonPoints[i].y,
                             polygonPoints[i+1].x, polygonPoints[i+1].y);
        }
    }

    // Рисуем точки полигона (синие с номером)
    painter.setPen(Qt::black);
    painter.setBrush(Qt::blue);
    for(int i=0;i<polygonPoints.size();++i){
        const auto &p = polygonPoints[i];
        painter.drawEllipse(QPointF(p.x, p.y), 6, 6);
        painter.drawText(p.x + 10, p.y - 10, QString::number(i+1));
    }

    // Рисуем тестовые точки с цветом по положению и подписью
    for(int i=0;i<extraPoints.size();++i){
        const auto &p = extraPoints[i];
        QColor pointColor = Qt::red;
        QString positionText = "P";

        if(hullBuilt){
            PointPosition pos = pointInPolygon(p, std::vector<Point>(hull.begin(), hull.end()), delta);
            pointColor = getColorForPosition(pos);
            positionText = getStatusText(pos);

            // Отображаем статус для последней точки
            if(i == extraPoints.size()-1){
                painter.setPen(Qt::black);
                painter.drawText(p.x + 15, p.y - 15, positionText);
            }
        }

        painter.setPen(Qt::black);
        painter.setBrush(pointColor);
        painter.drawEllipse(QPointF(p.x, p.y), 8, 8);
        painter.drawText(p.x - 5, p.y - 10, QString("P%1").arg(i+1));
    }

    // Легенда
    painter.setPen(Qt::black);
    painter.drawText(20, height() - 100, "Легенда:");
    painter.setBrush(Qt::blue);
    painter.drawEllipse(20, height() - 80, 8, 8);
    painter.drawText(40, height() - 73, "- точки полигона");

    if(hullBuilt){
        painter.setBrush(QColor(0, 255, 0));
        painter.drawEllipse(20, height() - 60, 8, 8);
        painter.drawText(40, height() - 53, "- точка внутри");

        painter.setBrush(Qt::red);
        painter.drawEllipse(20, height() - 40, 8, 8);
        painter.drawText(40, height() - 33, "- точка снаружи");

        painter.setBrush(QColor(255, 255, 0));
        painter.drawEllipse(200, height() - 60, 8, 8);
        painter.drawText(220, height() - 53, "- точка близко к границе");

        painter.setBrush(Qt::blue);
        painter.drawEllipse(200, height() - 40, 8, 8);
        painter.drawText(220, height() - 33, "- точка на границе");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event){
    QPointF pos = event->localPos();
    draggedIndex = -1;
    draggingPolygonPoint = false;

    // Проверяем нажатие на точки полигона
    for(int i=0;i<polygonPoints.size();++i){
        double d2 = (polygonPoints[i].x - pos.x())*(polygonPoints[i].x - pos.x()) +
                    (polygonPoints[i].y - pos.y())*(polygonPoints[i].y - pos.y());
        if(d2 < 64.0){ // радиус 8 пикселей
            draggedIndex = i;
            draggingPolygonPoint = true;
            statusBar()->showMessage(QString("Перетаскиваете точку полигона #%1").arg(i+1));
            return;
        }
    }

    // Проверяем нажатие на тестовые точки
    for(int i=0;i<extraPoints.size();++i){
        double d2 = (extraPoints[i].x - pos.x())*(extraPoints[i].x - pos.x()) +
                    (extraPoints[i].y - pos.y())*(extraPoints[i].y - pos.y());
        if(d2 < 64.0){
            draggedIndex = i;
            draggingPolygonPoint = false;
            statusBar()->showMessage(QString("Перетаскиваете тестовую точку P%1").arg(i+1));
            return;
        }
    }

    // Если клик не на существующих точках
    if(event->button() == Qt::LeftButton){
        bool exists = false;
        for(const auto &p: polygonPoints)
            if((p.x - pos.x())*(p.x - pos.x()) + (p.y - pos.y())*(p.y - pos.y()) < 36.0) exists = true;
        for(const auto &p: extraPoints)
            if((p.x - pos.x())*(p.x - pos.x()) + (p.y - pos.y())*(p.y - pos.y()) < 36.0) exists = true;

        if(!exists){
            if(!hullBuilt){
                polygonPoints.append(Point(pos.x(), pos.y()));
                statusLabel->setText(QString("Точка %1 добавлена. Всего точек: %2. Добавьте ещё или постройте оболочку.")
                                         .arg(polygonPoints.size())
                                         .arg(polygonPoints.size()));
                statusBar()->showMessage(QString("Добавлена точка #%1").arg(polygonPoints.size()));
            } else {
                extraPoints.append(Point(pos.x(), pos.y()));
                statusBar()->showMessage(QString("Добавлена тестовая точка P%1").arg(extraPoints.size()));

                // Обновляем статус
                if(!extraPoints.isEmpty()){
                    PointPosition posEnum = pointInPolygon(extraPoints.last(),
                                                           std::vector<Point>(hull.begin(), hull.end()),
                                                           delta);
                    statusLabel->setText(getStatusText(posEnum));
                    QString colorStyle;
                    switch(posEnum){
                    case PointPosition::Inside: colorStyle = "background: #d4edda; border-color: #c3e6cb; color: #155724;"; break;
                    case PointPosition::Outside: colorStyle = "background: #f8d7da; border-color: #f5c6cb; color: #721c24;"; break;
                    case PointPosition::OnBoundary: colorStyle = "background: #cce5ff; border-color: #b8daff; color: #004085;"; break;
                    case PointPosition::NearBoundary: colorStyle = "background: #fff3cd; border-color: #ffeeba; color: #856404;"; break;
                    }
                    statusLabel->setStyleSheet(QString("QLabel { padding: 5px; %1 }").arg(colorStyle));
                }
            }
            update();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if(draggedIndex == -1) return;

    QPointF pos = event->localPos();
    if(draggingPolygonPoint){
        polygonPoints[draggedIndex] = Point(pos.x(), pos.y());
        if(hullBuilt){
            rebuildHull();
            rebuildDelta();
            // Обновляем статус тестовых точек
            if(!extraPoints.isEmpty()){
                PointPosition posEnum = pointInPolygon(extraPoints.last(),
                                                       std::vector<Point>(hull.begin(), hull.end()),
                                                       delta);
                statusLabel->setText(getStatusText(posEnum));
            }
        }
    } else {
        extraPoints[draggedIndex] = Point(pos.x(), pos.y());
        if(hullBuilt && !extraPoints.isEmpty()){
            PointPosition posEnum = pointInPolygon(extraPoints[draggedIndex],
                                                   std::vector<Point>(hull.begin(), hull.end()),
                                                   delta);
            statusLabel->setText(getStatusText(posEnum));
        }
    }
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    Q_UNUSED(event);
    if(draggedIndex != -1){
        statusBar()->showMessage("Перетаскивание завершено");
    }
    draggedIndex = -1;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event){
    Q_UNUSED(event);
    if(!hullBuilt && polygonPoints.size() >= 3){
        buildConvexHull();
    }
}
