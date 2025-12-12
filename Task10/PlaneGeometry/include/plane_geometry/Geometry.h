#pragma once
#include <vector>
#include <optional>
#include <cmath>

namespace PlaneGeometry {

struct Point {
    double x{}, y{};
};

using Polygon = std::vector<Point>;

inline Point operator-(const Point& a, const Point& b) { return {a.x-b.x, a.y-b.y}; }
inline double cross(const Point& a, const Point& b) { return a.x*b.y - a.y*b.x; }
inline double cross(const Point& o, const Point& a, const Point& b) { return cross(a-o, b-o); }
inline double dot(const Point& a, const Point& b) { return a.x*b.x + a.y*b.y; }

Polygon convexHull(const std::vector<Point>& pts);

Polygon clipHalfPlane(const Polygon& subject, const Point& A, const Point& B, bool keepLeft);

Polygon intersectConvex(const Polygon& A, const Polygon& B);


Polygon differenceConvex(const Polygon& A, const Polygon& B);

std::vector<Polygon> unionConvexDecomposed(const Polygon& A, const Polygon& B);

bool pointInConvex(const Polygon& poly, const Point& p);

} 
