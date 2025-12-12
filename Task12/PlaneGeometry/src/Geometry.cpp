#include <algorithm>
#include <cmath>
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

// Убираем namespace Geometry и делаем функции глобальными

// ------------------ convexHull ------------------
std::vector<Point> convexHull(std::vector<Point> points) {
    if(points.size() <= 1) return points;

    std::sort(points.begin(), points.end(), [](const Point &a, const Point &b){
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    std::vector<Point> lower, upper;

    for(const auto &p : points){
        while(lower.size() >= 2){
            Point q = lower[lower.size()-2];
            Point r = lower[lower.size()-1];
            if((r.x - q.x)*(p.y - q.y) - (r.y - q.y)*(p.x - q.x) <= 0)
                lower.pop_back();
            else break;
        }
        lower.push_back(p);
    }

    for(auto it = points.rbegin(); it != points.rend(); ++it){
        const Point &p = *it;
        while(upper.size() >= 2){
            Point q = upper[upper.size()-2];
            Point r = upper[upper.size()-1];
            if((r.x - q.x)*(p.y - q.y) - (r.y - q.y)*(p.x - q.x) <= 0)
                upper.pop_back();
            else break;
        }
        upper.push_back(p);
    }

    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

// ------------------ distancePointToSegment ------------------
inline double distancePointToSegment(const Point &p, const Point &a, const Point &b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    if(dx==0 && dy==0) return std::hypot(p.x - a.x, p.y - a.y);
    double t = ((p.x - a.x)*dx + (p.y - a.y)*dy)/(dx*dx + dy*dy);
    t = std::max(0.0, std::min(1.0, t));
    double px = a.x + t*dx;
    double py = a.y + t*dy;
    return std::hypot(p.x - px, p.y - py);
}

// ------------------ Winding number для одного контура ------------------
inline int windingNumber(const Point &p, const std::vector<Point> &poly) {
    int wn = 0;
    int n = poly.size();
    for(int i=0;i<n;i++){
        const Point &pi = poly[i];
        const Point &pj = poly[(i+1)%n];
        if(pi.y <= p.y){
            if(pj.y > p.y && (pj.x - pi.x)*(p.y - pi.y) - (p.x - pi.x)*(pj.y - pi.y) > 0)
                wn++;
        } else {
            if(pj.y <= p.y && (pj.x - pi.x)*(p.y - pi.y) - (p.x - pi.x)*(pj.y - pi.y) < 0)
                wn--;
        }
    }
    return wn;
}

// ------------------ Проверка на границу ------------------
inline bool onBoundary(const Point &p, const std::vector<Point> &poly, double delta){
    int n = poly.size();
    for(int i=0;i<n;i++){
        if(distancePointToSegment(p, poly[i], poly[(i+1)%n]) < delta)
            return true;
    }
    return false;
}

// ------------------ pointInPolygon для полигона с дырками ------------------
PointPosition pointInPolygon(const Point &p, const std::vector<std::vector<Point>> &polygons, double delta){
    if(polygons.empty()) return PointPosition::Outside;

    // проверка границы всех контуров
    for(const auto &poly : polygons){
        if(onBoundary(p, poly, delta)) return PointPosition::NearBoundary;
    }

    // проверка внешнего контура
    int wn = windingNumber(p, polygons[0]);
    if(wn == 0) return PointPosition::Outside;

    // проверка дырок
    for(size_t i=1;i<polygons.size();i++){
        if(windingNumber(p, polygons[i]) != 0) return PointPosition::Outside;
    }

    return PointPosition::Inside;
}

// ------------------ minDistance ------------------
double minDistance(const std::vector<Point> &points){
    double minD = 1e9;
    int n = points.size();
    for(int i=0;i<n;i++){
        for(int j=i+1;j<n;j++){
            double d = std::hypot(points[i].x - points[j].x, points[i].y - points[j].y);
            if(d<minD) minD = d;
        }
    }
    return minD;
}
