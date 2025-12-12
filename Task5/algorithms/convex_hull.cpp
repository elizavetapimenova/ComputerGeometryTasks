#include "convex_hull.hpp"
#include <vector>
#include <algorithm>

std::vector<Point> ConvexHull::compute(const std::vector<Point>& points) {
    int n = points.size();
    if (n < 3) {
        return points;
    }

    std::vector<Point> sorted_points = points;
    std::sort(sorted_points.begin(), sorted_points.end());

    std::vector<Point> lower;
    for (int i = 0; i < n; ++i) {
        while (lower.size() >= 2) {
            Point& a = lower[lower.size() - 2];
            Point& b = lower[lower.size() - 1];
            if (cross(a, b, sorted_points[i]) <= 0) {
                lower.pop_back();
            } else {
                break;
            }
        }
        lower.push_back(sorted_points[i]);
    }

    std::vector<Point> upper;
    for (int i = n - 1; i >= 0; --i) {
        while (upper.size() >= 2) {
            Point& a = upper[upper.size() - 2];
            Point& b = upper[upper.size() - 1];
            if (cross(a, b, sorted_points[i]) <= 0) {
                upper.pop_back();
            } else {
                break;
            }
        }
        upper.push_back(sorted_points[i]);
    }

    lower.pop_back();
    upper.pop_back();

    std::vector<Point> hull = lower;
    hull.insert(hull.end(), upper.begin(), upper.end());

    return hull;
}
