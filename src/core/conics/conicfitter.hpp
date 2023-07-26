#pragma once

#include "src/core/settings.hpp"
#include <Eigen/Core>
#include "util/vector.hpp"

class ConicFitter {
public:
    explicit ConicFitter();

    Eigen::VectorX<long double> fitConic(const std::vector<PatchPoint> &patchPoints);

    double stability();

private:
    long double stability_ = 0;
    int numEq_ = 0;

    int numUnknowns_ = 0;

    [[nodiscard]] Eigen::Matrix<long double, Eigen::Dynamic, Eigen::Dynamic> initAEigen(const std::vector<PatchPoint> &patchPoints) const;

    Eigen::VectorX<long double> solveLinSystem(const Eigen::MatrixX<long double> &A);
};