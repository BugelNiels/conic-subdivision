#pragma once
#include "conis/core/vector.hpp"

#include <utility>

namespace conis::core::test {
using PointNormalPairs = std::pair<std::vector<Vector2DD>, std::vector<Vector2DD>>;

PointNormalPairs straightLine(int numPoints, real_t a, real_t b);
PointNormalPairs circle(int numPoints, real_t x, real_t y, real_t radius);
PointNormalPairs ellipse(int numPoints, real_t x, real_t y, real_t a, real_t b);
PointNormalPairs hyperbolaSingleBranch(int numPoints); // 4x^2-y^2=4
PointNormalPairs parabola(int numPoints);              // 2x^2 - 2y = 0
} // namespace conis::core::test
