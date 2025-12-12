// Убираем: #include "Core/Geometry.h"
#include <vector>
#include <cmath>
#include <algorithm>

struct Point {
    double x, y;
    Point(double x_ = 0, double y_ = 0) : x(x_), y(y_) {}

    // Для сравнения точек
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Triangle {
    Point a, b, c;
    Triangle(Point a_ = Point(), Point b_ = Point(), Point c_ = Point())
        : a(a_), b(b_), c(c_) {}

    // Для сравнения треугольников
    bool operator==(const Triangle& other) const {
        return (a == other.a && b == other.b && c == other.c);
    }
};

struct Circle {
    Point center;
    double radius;
};

static double dist2(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return dx*dx + dy*dy;
}

static Circle circumCircle(const Point& a, const Point& b, const Point& c) {
    double d = 2*(a.x*(b.y-c.y) + b.x*(c.y-a.y) + c.x*(a.y-b.y));

    // Проверка на коллинеарность (чтобы избежать деления на 0)
    if (std::abs(d) < 1e-10) {
        // Возвращаем фиктивный круг большого радиуса
        return {Point((a.x + b.x + c.x)/3, (a.y + b.y + c.y)/3), 1e10};
    }

    double ux = ((a.x*a.x + a.y*a.y)*(b.y - c.y) +
                 (b.x*b.x + b.y*b.y)*(c.y - a.y) +
                 (c.x*c.x + c.y*c.y)*(a.y - b.y)) / d;
    double uy = ((a.x*a.x + a.y*a.y)*(c.x - b.x) +
                 (b.x*b.x + b.y*b.y)*(a.x - c.x) +
                 (c.x*c.x + c.y*c.y)*(b.x - a.x)) / d;
    Point center{ux, uy};
    double radius = std::sqrt(dist2(center, a));
    return {center, radius};
}

// Убираем Geometry:: из имени функции
std::vector<Triangle> delaunayTriangulation(const std::vector<Point>& points) {
    std::vector<Triangle> triangles;
    if (points.size() < 3) return triangles;

    // Super-triangle
    double minX = points[0].x, maxX = points[0].x;
    double minY = points[0].y, maxY = points[0].y;
    for (const auto &p : points) {  // Добавляем const
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }
    double dx = maxX - minX;
    double dy = maxY - minY;

    // Увеличиваем супер-треугольник для надежности
    double margin = 100.0;  // Запас в 100 пикселей
    Point p1{minX - dx - margin, minY - dy - margin};
    Point p2{maxX + dx + margin, minY - dy - margin};
    Point p3{(minX + maxX)/2, maxY + dy + margin};

    triangles.push_back({p1, p2, p3});

    // Алгоритм Bowyer-Watson
    for (const auto &p : points) {  // Добавляем const
        std::vector<Triangle> badTriangles;

        // Находим "плохие" треугольники
        for (const auto &t : triangles) {
            Circle c = circumCircle(t.a, t.b, t.c);
            if (dist2(c.center, p) < c.radius*c.radius) {
                badTriangles.push_back(t);
            }
        }

        // Находим границу многоугольника
        std::vector<std::pair<Point, Point>> polygon;
        for (size_t i = 0; i < badTriangles.size(); ++i) {
            const auto& bt = badTriangles[i];
            std::pair<Point, Point> edges[3] = {
                {bt.a, bt.b}, {bt.b, bt.c}, {bt.c, bt.a}
            };

            for (const auto& e : edges) {
                bool shared = false;

                // Проверяем, делится ли ребро с другим плохим треугольником
                for (size_t j = 0; j < badTriangles.size(); ++j) {
                    if (i == j) continue;

                    const auto& bt2 = badTriangles[j];
                    std::pair<Point, Point> edges2[3] = {
                        {bt2.a, bt2.b}, {bt2.b, bt2.c}, {bt2.c, bt2.a}
                    };

                    for (const auto& e2 : edges2) {
                        if ((e.first == e2.first && e.second == e2.second) ||
                            (e.first == e2.second && e.second == e2.first)) {
                            shared = true;
                            break;
                        }
                    }
                    if (shared) break;
                }

                if (!shared) {
                    polygon.push_back(e);
                }
            }
        }

        // Удаляем плохие треугольники
        triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
                                       [&](const Triangle& t) {
                                           return std::find(badTriangles.begin(), badTriangles.end(), t) != badTriangles.end();
                                       }), triangles.end());

        // Добавляем новые треугольники
        for (const auto& edge : polygon) {
            triangles.push_back({edge.first, edge.second, p});
        }
    }

    // Удаляем треугольники, содержащие вершины супер-треугольника
    triangles.erase(std::remove_if(triangles.begin(), triangles.end(),
                                   [&](const Triangle& t) {
                                       return (dist2(t.a, p1) < 1e-6 || dist2(t.a, p2) < 1e-6 || dist2(t.a, p3) < 1e-6 ||
                                               dist2(t.b, p1) < 1e-6 || dist2(t.b, p2) < 1e-6 || dist2(t.b, p3) < 1e-6 ||
                                               dist2(t.c, p1) < 1e-6 || dist2(t.c, p2) < 1e-6 || dist2(t.c, p3) < 1e-6);
                                   }), triangles.end());

    return triangles;
}
