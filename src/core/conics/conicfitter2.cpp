#include "conicfitter2.hpp"
#include <Eigen/SVD>
#include <Eigen/Eigen>
#include <ceres/ceres.h>
#include <iostream>

struct CostFunction {
    CostFunction(const Eigen::MatrixX<double> &A) : A_(A) {}

    template<typename T>
    bool operator()(const T *const x, T *residual) const {
        for (int i = 0; i < A_.rows(); ++i) {
            residual[i] = T(0);
            for (int j = 0; j < A_.cols(); ++j) {
                residual[i] += A_(i, j) * x[j];
            }
        }
        return true;
    }

    const Eigen::MatrixX<double> &A_;
};

ConicFitter2::ConicFitter2() {
    options_.linear_solver_type = ceres::DENSE_QR;
    options_.max_num_iterations = 100;
    options_.function_tolerance = 1e-8;
    options_.gradient_tolerance = 1e-8;
    options_.parameter_tolerance = 1e-8;
//    options.minimizer_progress_to_stdout = true;
}

static Eigen::RowVectorXd pointEqEigen(const Vector2DD &coord, int numUnknowns) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    long double x = coord.x();
    long double y = coord.y();
    row(0) = x * x;
    row(1) = y * y;
    row(2) = 2 * x * y;
    row(3) = 2 * x;
    row(4) = 2 * y;
    row(5) = 1;
    return row;
}

static Eigen::RowVectorXd normEqXEigen(const Vector2DD &coord,
                                       const Vector2DD &normal,
                                       int numUnknowns,
                                       int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    long double x = coord.x();
    long double y = coord.y();
    row(0) = 2 * x;  // A
    row(1) = 0;      // B
    row(2) = 2 * y;  // C
    row(3) = 2;      // D
    row(4) = 0;      // E
    row(5) = 0;      // F
    row(6 + normIdx) = -normal.x();
    return row;
}

static Eigen::RowVectorXd normEqYEigen(const Vector2DD &coord,
                                       const Vector2DD &normal,
                                       int numUnknowns,
                                       int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    long double x = coord.x();
    long double y = coord.y();
    row(0) = 0;        // A
    row(1) = 2 * y;    // B
    row(2) = 2 * x;    // C
    row(3) = 0;        // E
    row(4) = 2;        // F
    row(5) = 0;        // G
    row(6 + normIdx) = -normal.y();
    return row;
}

Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>
ConicFitter2::initAEigen(const std::vector<PatchPoint> &patchPoints) const {
    Eigen::MatrixXd A(numEq_, numUnknowns_);

    int rowIdx = 0;
    for (int i = 0; i < patchPoints.size(); i++) {
        auto &p = patchPoints[i];
        const Vector2DD &coord = p.coords;
        A.row(rowIdx++) = pointEqEigen(coord, numUnknowns_) * p.pointWeight;
    }

    for (int i = 0; i < patchPoints.size(); i++) {
        auto &p = patchPoints[i];
        const Vector2DD &coord = p.coords;
        const Vector2DD &normal = p.normal;
        A.row(rowIdx++) = normEqXEigen(coord, normal, numUnknowns_, i) * p.normWeight;
        A.row(rowIdx++) = normEqYEigen(coord, normal, numUnknowns_, i) * p.normWeight;
    }
    return A;
}

double ConicFitter2::solutionSolve(const Eigen::MatrixXd &A, Eigen::VectorXd &solution, int initGuess,
                                   Eigen::VectorXd &guess) {
    ceres::Problem problem;

    double *x = new double[numUnknowns_];
    for (int i = 0; i < numUnknowns_; i++) {
        x[i] = guess[i];
        if (i > 5) {
            x[i] = initGuess;
        }
    }

    CostFunction *costFunctor = new CostFunction(A);
    ceres::CostFunction *costFunc = new ceres::AutoDiffCostFunction<CostFunction, 12, 10>(costFunctor);
    problem.AddResidualBlock(costFunc, nullptr, x);

    if (initGuess > 0) {
        for (int i = 6; i < numUnknowns_; i++) {
            problem.SetParameterLowerBound(x, i, 1);
        }
    } else {
        for (int i = 6; i < numUnknowns_; i++) {
            problem.SetParameterUpperBound(x, i, -1);
        }
    }


    ceres::Solver::Summary summary;
    ceres::Solve(options_, &problem, &summary);

#if 0
    std::cout << summary.BriefReport() << std::endl;
    for (int i = 0; i < numUnknowns_; ++i) {
        solution(i) = x[i];
        std::cout << x[i] << " ";
    }
    std::cout << std::endl;
#endif
    delete[] x;
    return summary.final_cost;
}


Eigen::VectorXd ConicFitter2::solveLinSystem(const Eigen::MatrixXd &A) {
    // TODO: use Ceres, minimise objective function such that the signs of all normal scaling factors are equal
    // Might as well introduce the equality constraints for both the points and normals then

    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeThinV);
    const auto &V = svd.matrixV();
    stability_ = svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size() - 1);
    Eigen::VectorXd guess = V.col(V.cols() - 1);

    Eigen::VectorXd posSolution(numUnknowns_);
    double posErr = solutionSolve(A, posSolution, 1, guess);

    Eigen::VectorXd negSolution(numUnknowns_);
    double negErr = solutionSolve(A, negSolution, -1, guess);

    if (posErr > negErr) {
        return negSolution;
    }
    return posSolution;
}

Eigen::VectorXd ConicFitter2::fitConic(const std::vector<PatchPoint> &patchPoints) {
    int numPoints = int(patchPoints.size());
    numUnknowns_ = 6 + numPoints;

    numEq_ = numPoints * 3; // 1 eq per coordinate + 2 per normal
    return solveLinSystem(initAEigen(patchPoints));
}

double ConicFitter2::stability() {
    double minVal = 10.0 * 10000;
    double maxVal = 1.0e9 * minVal;
    double logMin = std::log10(minVal);
    double logMax = std::log10(maxVal);

    double logValue = std::log10(stability_);

    double mappedValue = (logValue - logMin) / (logMax - logMin);
    return std::clamp(mappedValue, 0.0, 1.0);
}
