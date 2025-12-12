#pragma once
#include <vector>
#include <cmath>

struct Point {
    double x, y;
    Point(double x_=0, double y_=0) : x(x_), y(y_) {}
    
    Point operator-(const Point &p) const { return Point(x - p.x, y - p.y); }
    double cross(const Point &p) const { return x * p.y - y * p.x; }
    double dist2(const Point &p) const { return (x - p.x)*(x - p.x) + (y - p.y)*(y - p.y); }
};
