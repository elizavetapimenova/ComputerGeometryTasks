#pragma once
#include <QMainWindow>
#include <QVector>
#include <QMouseEvent>
#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <cmath>

struct Point {
    double x, y;

    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}

    Point operator-(const Point& other) const {
        return Point(x - other.x, y - other.y);
    }

    double dist2(const Point& other) const {
        double dx = x - other.x;
        double dy = y - other.y;
        return dx*dx + dy*dy;
    }

    double cross(const Point& other) const {
        return x * other.y - y * other.x;
    }
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
    void buildConvexHull();
    void clearAll();

private:
    // Геометрические функции (теперь здесь)
    std::vector<Point> convexHull(std::vector<Point> points);
    double minDistance(const std::vector<Point>& polygon);
    PointPosition pointInPolygon(const Point &p, const std::vector<Point> &polygon, double delta);

    void rebuildDelta();
    void rebuildHull();
    QColor getColorForPosition(PointPosition position);
    QString getStatusText(PointPosition position);

    QVector<Point> polygonPoints;     // исходные точки полигона
    QVector<Point> hull;              // вершины выпуклой оболочки
    QVector<Point> extraPoints;       // тестовые точки
    bool hullBuilt;
    double delta;
    int draggedIndex;
    bool draggingPolygonPoint;

    QLabel *statusLabel;
    QPushButton *buildButton;
    QPushButton *clearButton;
    QWidget *centralWidget;
};
