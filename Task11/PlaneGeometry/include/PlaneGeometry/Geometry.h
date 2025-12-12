#pragma once
#include "Point.h"
#include <vector>

enum class PointPosition { Inside, Outside, OnBoundary, NearBoundary };

class Geometry {
public:
    static std::vector<Point> convexHull(std::vector<Point> points);
    static PointPosition pointInPolygon(const Point &p, const std::vector<Point> &polygon, double delta);
    static double minDistance(const std::vector<Point> &polygon);
};
