#pragma once
#include <vector>
#include <cmath>

struct Point {
    double x, y;
    Point(double xx=0,double yy=0) : x(xx), y(yy) {}
};

enum class PointPosition { Inside, Outside, NearBoundary };

namespace Geometry {

// convexHull алгоритм Andrew’s monotone chain
std::vector<Point> convexHull(std::vector<Point> points);

// Проверка положения точки относительно полигона с дырками
PointPosition pointInPolygon(const Point &p, const std::vector<std::vector<Point>> &polygons, double delta);

// Минимальное расстояние между точками
double minDistance(const std::vector<Point> &points);

} // namespace Geometry
