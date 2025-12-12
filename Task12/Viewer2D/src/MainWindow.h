#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPointF>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <vector>

struct Point {
    double x, y;

    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}
};

enum class PointPosition {
    Inside,
    Outside,
    OnBoundary,
    NearBoundary
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent=nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void buildPolygon();
    void addHole();
    void clearAll();
    void buildConvexHull();

private:
    QVector<Point> currentContour;         // текущий контур (основной или дырка)
    QVector<QVector<Point>> polygons;      // все полигоны: [0] - основной, остальные - дырки
    QVector<Point> testPoints;             // тестовые точки
    QVector<Point> convexHullPoints;       // точки выпуклой оболочки

    bool polygonBuilt = false;             // построен ли основной полигон
    bool creatingHole = false;             // создаем ли дырку сейчас
    double delta = 5.0;                    // для проверки границы
    int draggedPointIndex = -1;            // индекс перетаскиваемой тестовой точки
    bool dragging = false;

    QLabel *statusLabel;
    QPushButton *buildPolygonBtn;
    QPushButton *addHoleBtn;
    QPushButton *clearBtn;
    QPushButton *convexHullBtn;

    void rebuildDelta();
    QColor getColorForPosition(PointPosition pos);
    QString getStatusText(PointPosition pos);
    void updateTestPointStatus();
};

// Объявляем геометрические функции (вне класса!)
std::vector<Point> convexHull(std::vector<Point> points);
double minDistance(const std::vector<Point>& points);
PointPosition pointInPolygon(const Point& p, const std::vector<std::vector<Point>>& polygons, double delta);

#endif // MAINWINDOW_H
