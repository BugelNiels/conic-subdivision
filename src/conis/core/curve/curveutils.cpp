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
    auto ab = a - b;
    auto cb = c - b;
    real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
    return cross > 0 ? -1 * normal : normal;
}

Vector2DD CurveUtils::calcNormalOscCircles(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) {
    if (a == b) {
        Vector2DD normal = c - b;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    } else if (b == c) {
        Vector2DD normal = b - a;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    } else {
        real_t d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) + c.x() * (a.y() - b.y()));
        real_t ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                     (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                     (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                    d;
        real_t uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                     (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                     (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                    d;
        Vector2DD oscCircleCenter = Vector2DD(ux, uy);
        Vector2DD norm = (oscCircleCenter - b).normalized();

        Vector2DD check = CurveUtils::calcNormal(a, b, c, false);
        if (check.dot(norm) < 0) {
            norm *= -1;
        }
        return norm;
    }
}

real_t CurveUtils::distanceToEdge(const Vector2DD &a, const Vector2DD &b, const Vector2DD &p) {
    Vector2DD ab = b - a;
    Vector2DD ap = p - a;
    real_t ab_len2 = ab.squaredNorm();
    real_t t = ap.dot(ab) / ab_len2;
    t = std::max(real_t(0.0), std::min(real_t(1.0), t)); // clamp to [0, 1]
    Vector2DD q = a + t * ab;
    return std::hypot(p.x() - q.x(), p.y() - q.y());
}

real_t CurveUtils::calcCurvature(const Vector2DD &a,
                                 const Vector2DD &b,
                                 const Vector2DD &c,
                                 CurvatureType curvatureType) {
    if (curvatureType == CurvatureType::CIRCLE_RADIUS) {
        Vector2DD ab = a - b;
        Vector2DD cb = c - b;
        Vector2DD ac = a - c;

        real_t denom = ab.dot(ab) * cb.dot(cb) * ac.dot(ac);

        // Avoid division by zero
        if (denom == 0.0)
            return 0.0;
        real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
        return sqrt((cross * cross) / denom);
    }

    const auto e1 = b - a;
    const auto e_1 = c - a;
    real_t cross = e_1.x() * e1.y() - e_1.y() * e1.x();
    real_t dot = e_1.x() * e1.x() + e_1.y() * e1.y();
    real_t v = atan(cross / dot);

    real_t denom = e_1.norm() + e1.norm();
    if (denom == 0.0)
        return 0.0;

    if (curvatureType == CurvatureType::DISCRETE_WINDING) {
        return 2.0 * v / denom;
    } else if (curvatureType == CurvatureType::GRADIENT_ARC_LENGTH) {
        return 4.0 * sin(v / 2.0) / denom;
    } else if (curvatureType == CurvatureType::AREA_INFLATION) {
        return 4.0 * tan(v / 2.0) / denom;
    }
    std::cerr << "Unsupported curvature type: " << curvatureType;
    return 0; // Unsupported curvature type
}

} // namespace conis::core