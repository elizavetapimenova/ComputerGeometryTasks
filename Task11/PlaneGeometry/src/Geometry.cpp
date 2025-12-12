#include "PlaneGeometry/Geometry.h"
#include <algorithm>
#include <limits>

using namespace std;

// Алгоритм Грэхема / сортировка по углу и выпуклая оболочка
vector<Point> Geometry::convexHull(vector<Point> points) {
    if(points.size() <= 3) return points;
    sort(points.begin(), points.end(), [](const Point &a, const Point &b){
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    vector<Point> hull;
    // Нижняя
    for(auto &p: points) {
        while(hull.size() >= 2 && ((hull.back()-hull[hull.size()-2]).cross(p-hull.back())) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    // Верхняя
    size_t t = hull.size() + 1;
    for(int i = points.size()-2; i>=0; --i){
        auto &p = points[i];
        while(hull.size() >= t && ((hull.back()-hull[hull.size()-2]).cross(p-hull.back())) <=0)
            hull.pop_back();
        hull.push_back(p);
    }
    hull.pop_back();
    return hull;
}

// Минимальное расстояние между точками полигона
double Geometry::minDistance(const vector<Point> &polygon){
    double minDist = numeric_limits<double>::max();
    for(size_t i=0;i<polygon.size();++i){
        for(size_t j=i+1;j<polygon.size();++j){
            double d = sqrt(polygon[i].dist2(polygon[j]));
            if(d < minDist) minDist = d;
        }
    }
    return minDist;
}

// Проверка точки относительно полигона (лучевая проверка)
PointPosition Geometry::pointInPolygon(const Point &p, const vector<Point> &polygon, double delta){
    bool inside = false;
    size_t n = polygon.size();
    for(size_t i=0,j=n-1;i<n;j=i++){
        Point pi = polygon[i], pj = polygon[j];
        if( ((pi.y > p.y) != (pj.y > p.y)) &&
            (p.x < (pj.x - pi.x) * (p.y - pi.y)/(pj.y - pi.y) + pi.x) )
            inside = !inside;
        // Проверка близости к границе
        double dx = pj.x - pi.x, dy = pj.y - pi.y;
        double t = ((p.x - pi.x)*dx + (p.y - pi.y)*dy)/(dx*dx + dy*dy);
        if(t>=0 && t<=1){
            double px = pi.x + t*dx;
            double py = pi.y + t*dy;
            double d2 = (p.x - px)*(p.x - px) + (p.y - py)*(p.y - py);
            if(d2 < delta*delta) return PointPosition::NearBoundary;
        }
    }
    return inside ? PointPosition::Inside : PointPosition::Outside;
}
