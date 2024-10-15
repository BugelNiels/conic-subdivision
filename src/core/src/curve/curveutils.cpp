#include "curveutils.hpp"

#include <iostream>

#include "conis/core/vector.hpp"

namespace conis::core {

Vector2DD CurveUtils::calcNormal(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c, bool areaWeighted) {
    if (a == b) {
        Vector2DD normal = c - b;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    if (b == c) {
        Vector2DD normal = b - a;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    Vector2DD t1 = (a - b);
    t1 = {-t1.y(), t1.x()};
    Vector2DD t2 = (b - c);
    t2 = {-t2.y(), t2.x()};
    if (!areaWeighted) {
        t1.normalize();
        t2.normalize();
    }
    Vector2DD normal = (t1 + t2).normalized();
    // Ensure correct orientation; normal is always pointing outwards
    const auto ab = a - b;
    const auto cb = c - b;
    const real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
    return cross > 0 ? -1 * normal : normal;
}

Vector2DD CurveUtils::calcNormalOscCircles(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) {
    if (a == b) {
        Vector2DD normal = c - b;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    if (b == c) {
        Vector2DD normal = b - a;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    const real_t d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) + c.x() * (a.y() - b.y()));
    const real_t ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                       (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                       (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                      d;
    const real_t uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                       (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                       (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                      d;
    const Vector2DD oscCircleCenter = {ux, uy};
    const Vector2DD norm = (oscCircleCenter - b).normalized();

    const Vector2DD check = calcNormal(a, b, c, false);
    if (check.dot(norm) < 0) {
        return norm * -1;
    }
    return norm;
}

real_t CurveUtils::distanceToEdge(const Vector2DD &a, const Vector2DD &b, const Vector2DD &p) {
    const Vector2DD ab = b - a;
    const Vector2DD ap = p - a;
    const real_t ab_len2 = ab.squaredNorm();
    real_t t = ap.dot(ab) / ab_len2;
    t = std::max(static_cast<real_t>(0.0), std::min(static_cast<real_t>(1.0), t)); // clamp to [0, 1]
    Vector2DD q = a + t * ab;
    return std::hypot(p.x() - q.x(), p.y() - q.y());
}

real_t CurveUtils::calcCurvature(const Vector2DD &a,
                                 const Vector2DD &b,
                                 const Vector2DD &c,
                                 const CurvatureType curvatureType) {
    if (curvatureType == CIRCLE_RADIUS) {
        const Vector2DD ab = a - b;
        const Vector2DD cb = c - b;
        const Vector2DD ac = a - c;

        const real_t denom = ab.dot(ab) * cb.dot(cb) * ac.dot(ac);

        // Avoid division by zero
        if (denom == 0.0)
            return 0.0;
        const real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
        return sqrt((cross * cross) / denom);
    }

    const auto e1 = b - a;
    const auto e_1 = c - a;
    const real_t cross = e_1.x() * e1.y() - e_1.y() * e1.x();
    const real_t dot = e_1.x() * e1.x() + e_1.y() * e1.y();
    const real_t v = atan(cross / dot);

    const real_t denom = e_1.norm() + e1.norm();
    if (denom == 0.0)
        return 0.0;

    if (curvatureType == DISCRETE_WINDING) {
        return 2.0 * v / denom;
    }
    if (curvatureType == GRADIENT_ARC_LENGTH) {
        return 4.0 * sin(v / 2.0) / denom;
    }
    if (curvatureType == AREA_INFLATION) {
        return 4.0 * tan(v / 2.0) / denom;
    }
    std::cerr << "Unsupported curvature type: " << curvatureType;
    return 0; // Unsupported curvature type
}

} // namespace conis::core