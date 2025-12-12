#pragma once
#include <QWidget>
#include <QVector>
#include <QPointF>

struct Point {
    double x, y;

    // Конструкторы
    Point() : x(0), y(0) {}
    Point(double x_, double y_) : x(x_), y(y_) {}
    Point(const QPointF& qpoint) : x(qpoint.x()), y(qpoint.y()) {}

    // Явно указываем, что хотим использовать сгенерированные методы
    Point(const Point&) = default;
    Point& operator=(const Point&) = default;

    bool operator==(const Point& other) const {
        return std::abs(x - other.x) < 1e-6 && std::abs(y - other.y) < 1e-6;
    }
};

struct Triangle {
    Point a, b, c;

    // Конструкторы
    Triangle() : a(), b(), c() {}
    Triangle(Point a_, Point b_, Point c_) : a(a_), b(b_), c(c_) {}

    // Явно указываем, что хотим использовать сгенерированные методы
    Triangle(const Triangle&) = default;
    Triangle& operator=(const Triangle&) = default;

    bool operator==(const Triangle& other) const {
        return (a == other.a && b == other.b && c == other.c);
    }
};

class DrawingWidget : public QWidget {
    Q_OBJECT
public:
    explicit DrawingWidget(QWidget *parent = nullptr);
    ~DrawingWidget() = default;

    // Удаляем копирование для QWidget
    DrawingWidget(const DrawingWidget&) = delete;
    DrawingWidget& operator=(const DrawingWidget&) = delete;

    void rebuildTriangulation();
    void clearPoints(); // Должен быть объявлен
    bool autoUpdate = false;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QVector<Point> points;
    QVector<Triangle> triangles;
    int draggingIndex;
};
