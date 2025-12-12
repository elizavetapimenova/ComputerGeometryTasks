#include "plane_geometry/Geometry.h"
#include <algorithm>
#include <cmath>

namespace PlaneGeometry {

static bool lexLess(const Point& a, const Point& b) {
    if (a.x == b.x) return a.y < b.y;
    return a.x < b.x;
}

Polygon convexHull(const std::vector<Point>& P) {
    std::vector<Point> pts = P;
    Polygon H;
    if (pts.size() < 3) return pts;

    std::sort(pts.begin(), pts.end(), lexLess);
    for (const auto& p : pts) {
        while (H.size() >= 2 && cross(H[H.size()-2], H.back(), p) <= 0) H.pop_back();
        H.push_back(p);
    }
    size_t t = H.size();
    for (int i = (int)pts.size()-2; i >= 0; --i) {
        const auto& p = pts[i];
        while (H.size() > t && cross(H[H.size()-2], H.back(), p) <= 0) H.pop_back();
        H.push_back(p);
    }
    if (!H.empty()) H.pop_back();

    return H;
}

static bool insideLeft(const Point& A, const Point& B, const Point& P) {
    return cross(A, B, P) >= -1e-12;
}
static bool insideRight(const Point& A, const Point& B, const Point& P) {
    return cross(A, B, P) <=  1e-12;
}

static bool segmentIntersectProper(const Point& a, const Point& b,
                                   const Point& c, const Point& d,
                                   Point& out)
{
    Point r = b - a, s = d - c;
    double rxs = cross(r, s);
    if (std::fabs(rxs) < 1e-15) return false; 
    double t = cross(c - a, s) / rxs;
    out = { a.x + t*r.x, a.y + t*r.y };
    return true;
}

Polygon clipHalfPlane(const Polygon& subject, const Point& A, const Point& B, bool keepLeft) {
    if (subject.empty()) return {};
    Polygon output;
    auto isIn = keepLeft ? insideLeft : insideRight;

    Point S = subject.back();
    bool Sin = isIn(A, B, S);

    for (const Point& E : subject) {
        bool Ein = isIn(A, B, E);

        if (Ein && Sin) {

            output.push_back(E);
        } else if (Sin && !Ein) {

            Point I;
            if (segmentIntersectProper(S, E, A, B, I)) output.push_back(I);
        } else if (!Sin && Ein) {
       
            Point I;
            if (segmentIntersectProper(S, E, A, B, I)) output.push_back(I);
            output.push_back(E);
        } 
        S = E; Sin = Ein;
    }
    return output;
}

Polygon intersectConvex(const Polygon& subject, const Polygon& clip) {
    if (subject.empty() || clip.empty()) return {};
    Polygon poly = subject;
    const int m = (int)clip.size();
    for (int i = 0; i < m && !poly.empty(); ++i) {
        const Point& A = clip[i];
        const Point& B = clip[(i+1)%m];
        poly = clipHalfPlane(poly, A, B, /*keepLeft=*/true);
    }
    return poly;
}

Polygon differenceConvex(const Polygon& A, const Polygon& B) {
    if (A.empty()) return {};
    if (B.empty()) return A;
    Polygon poly = A;
    const int m = (int)B.size();
    for (int i = 0; i < m && !poly.empty(); ++i) {
        const Point& P = B[i];
        const Point& Q = B[(i+1)%m];
        poly = clipHalfPlane(poly, P, Q, /*keepLeft=*/false); 
    }
    return poly;
}

std::vector<Polygon> unionConvexDecomposed(const Polygon& A, const Polygon& B) {
    std::vector<Polygon> parts;
    Polygon AminusB = differenceConvex(A, B);
    if (!AminusB.empty()) parts.push_back(std::move(AminusB));

    Polygon BminusA = differenceConvex(B, A);
    if (!BminusA.empty()) parts.push_back(std::move(BminusA));

    Polygon I = intersectConvex(A, B);
    if (!I.empty()) parts.push_back(std::move(I));

    return parts; 
}

bool pointInConvex(const Polygon& poly, const Point& p) {
    const int n = (int)poly.size();
    if (n < 3) return false;
    for (int i = 0; i < n; ++i) {
        if (cross(poly[i], poly[(i+1)%n], p) < -1e-12) return false;
    }
    return true;
}

static double signedArea(const Polygon& P) {
    double a = 0.0;
    const int n = (int)P.size();
    for (int i = 0; i < n; ++i) {
        const auto& p = P[i];
        const auto& q = P[(i+1)%n];
        a += p.x*q.y - p.y*q.x;
    }
    return 0.5 * a; // >0 => CCW, <0 => CW
}



static void ensureCCW(Polygon& P) {
    if (P.size() >= 3 && signedArea(P) < 0) std::reverse(P.begin(), P.end());
}


} 
