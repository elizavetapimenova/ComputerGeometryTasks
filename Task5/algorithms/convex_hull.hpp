#ifndef CONVEX_HULL_HPP
#define CONVEX_HULL_HPP

#include <vector>
#include <algorithm>
#include <QPointF>

struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    Point(const QPointF& p) : x(p.x()), y(p.y()) {}

    QPointF toQPointF() const { return QPointF(x, y); }

    bool operator<(const Point& p) const {
        return x < p.x || (x == p.x && y < p.y);
    }
};

class ConvexHull {
public:
    static std::vector<Point> compute(const std::vector<Point>& points);

    static double cross(const Point& a, const Point& b, const Point& c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }
};

#endif
