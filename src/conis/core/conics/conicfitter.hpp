#pragma once

#include <Eigen/Core>

#include "conis/core/vector.hpp"

namespace conis::core {

class ConicFitter {
public:
    explicit ConicFitter();

    Eigen::VectorX<real_t> fitConic(const std::vector<PatchPoint> &patchPoints);

    real_t stability() const;

private:
    real_t stability_ = 0;
    int numEq_ = 0;

    int numUnknowns_ = 0;

    [[nodiscard]] Eigen::Matrix<real_t, Eigen::Dynamic, Eigen::Dynamic> initAEigen(
            const std::vector<PatchPoint> &patchPoints) const;

    Eigen::VectorX<real_t> solveLinSystem(const Eigen::MatrixX<real_t> &A);
};

} // namespace conis::core