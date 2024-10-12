#pragma once
#include "conis/core/vector.hpp"
#include "conis/core/curve/curvaturetype.hpp"

namespace conis::core {

class CurveUtils {
public:
    static Vector2DD calcNormal(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c, bool areaWeighted = true);
    static Vector2DD calcNormalOscCircles(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c);
    static real_t distanceToEdge(const Vector2DD &a, const Vector2DD &b, const Vector2DD &p);
    // Calculates the curvature at point b for the segment a-b-c
    static real_t calcCurvature(const Vector2DD &a,
                                const Vector2DD &b,
                                const Vector2DD &c,
                                CurvatureType curvatureType);
};

} // namespace conis::core