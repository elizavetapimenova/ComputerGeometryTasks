#pragma once
#include <vector>

struct Point {
    double x;
    double y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Triangle {
    Point a;
    Point b;
    Point c;

    bool operator==(const Triangle& other) const {
        return a == other.a && b == other.b && c == other.c;
    }
};


class Geometry {
public:
    // Возвращает вектор треугольников для триангуляции Делоне
    static std::vector<Triangle> delaunayTriangulation(const std::vector<Point>& points);
};
