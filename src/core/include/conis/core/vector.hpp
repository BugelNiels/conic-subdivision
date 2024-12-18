#pragma once

#include <Eigen/Core>
#include <utility>

namespace conis::core {

#define EXTRA_CONIC_PRECISION
// #define NORMALIZE_CONIC_NORMALS
using real_t = long double;


using Vector2DD = Eigen::Vector2<real_t>;
using Vector3DD = Eigen::Vector3<real_t>;
using Vector4DD = Eigen::Vector4<real_t>;

using Matrix4DD = Eigen::Matrix4<real_t>;
using Matrix3DD = Eigen::Matrix3<real_t>;

using PatchPoint = struct PatchPoint {
    Vector2DD vertex;
    Vector2DD normal;
    real_t pointWeight;
    real_t normWeight;

    PatchPoint(Vector2DD vertex, Vector2DD normal, const real_t pointWeight, const real_t normWeight)
        : vertex(std::move(vertex)),
          normal(std::move(normal)),
          pointWeight(pointWeight),
          normWeight(normWeight) {}
};

template<typename T>
T mix(const T &a, const T &b, real_t w) {
    return (1.0 - w) * a + w * b;
}

} // namespace conis::core
