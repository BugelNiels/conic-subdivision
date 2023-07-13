#pragma once

#include <Eigen/Core>

using Vector2DD = Eigen::Vector2<long double>;
using Vector3DD = Eigen::Vector3<long double>;
using Vector4DD = Eigen::Vector4<long double>;

using Matrix4DD = Eigen::Matrix4<long double>;
using Matrix3DD = Eigen::Matrix3<long double>;


typedef struct PatchPoint {
    Vector2DD coords;
    Vector2DD normal;
    double pointWeight;
    double normWeight;
} PatchPoint;
