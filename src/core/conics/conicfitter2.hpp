#pragma once

#include <ceres/ceres.h>
#include "src/core/settings.hpp"
#include <Eigen/Core>
#include "util/vector.hpp"

class ConicFitter2 {
public:
    explicit ConicFitter2();

    Eigen::VectorXd fitConic(const std::vector<PatchPoint> &patchPoints);

    double stability();

private:
    long double stability_ = 0;
    int numEq_ = 0;

    ceres::Solver::Options options_;

    int numUnknowns_ = 0;

    [[nodiscard]] Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> initAEigen(const std::vector<PatchPoint> &patchPoints) const;
    double solutionSolve(const Eigen::MatrixXd &A, Eigen::VectorXd& solution, int initGuess, Eigen::VectorXd& guess);

    Eigen::VectorXd solveLinSystem(const Eigen::MatrixXd &A);
};