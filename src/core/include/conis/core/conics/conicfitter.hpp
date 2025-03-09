#pragma once

#include <Eigen/Core>

#include "conic.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

class ConicFitter {
public:
    explicit ConicFitter(real_t epsilon);

    Conic fitConic(const std::vector<PatchPoint> &patchPoints);

private:
    int numEq_ = 0;
    int numUnknowns_ = 0;
    real_t epsilon_;

    [[nodiscard]] Eigen::Matrix<real_t, Eigen::Dynamic, Eigen::Dynamic> initAEigen(
        const std::vector<PatchPoint> &patchPoints) const;

    Eigen::VectorX<real_t> solveLinSystem(const Eigen::MatrixX<real_t> &A);
};

} // namespace conis::core
