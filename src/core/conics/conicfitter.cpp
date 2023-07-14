#include "conicfitter.hpp"
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Eigen/Eigen>

ConicFitter::ConicFitter(const Settings &settings) : settings_(settings) {}

static Eigen::RowVectorX<long double> pointEqEigen(const Vector2DD &coord, int numUnknowns) {
    Eigen::RowVectorX<long double> row = Eigen::RowVectorX<long double>::Zero(numUnknowns);
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

static Eigen::RowVectorX<long double> normEqXEigen(const Vector2DD &coord,
                                                   const Vector2DD &normal,
                                                   int numUnknowns,
                                                   int normIdx) {
    Eigen::RowVectorX<long double> row = Eigen::RowVectorX<long double>::Zero(numUnknowns);
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

static Eigen::RowVectorX<long double> normEqYEigen(const Vector2DD &coord,
                                                   const Vector2DD &normal,
                                                   int numUnknowns,
                                                   int normIdx) {
    Eigen::RowVectorX<long double> row = Eigen::RowVectorX<long double>::Zero(numUnknowns);
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

Eigen::Matrix<long double, Eigen::Dynamic, Eigen::Dynamic>
ConicFitter::initAEigen(const std::vector<PatchPoint> &patchPoints) const {
    Eigen::MatrixX<long double> A(numEq_, numUnknowns_);

    int rowIdx = 0;
    for (int i = 0; i < patchPoints.size(); i++) {
        auto &p = patchPoints[i];
        const Vector2DD &coord = p.coords;
        const Vector2DD &normal = p.normal;
        A.row(rowIdx++) = pointEqEigen(coord, numUnknowns_) * p.pointWeight;
        A.row(rowIdx++) = normEqXEigen(coord, normal, numUnknowns_, i) * p.normWeight;
        A.row(rowIdx++) = normEqYEigen(coord, normal, numUnknowns_, i) * p.normWeight;
    }
    return A;
}

Eigen::VectorX<long double> ConicFitter::solveLinSystem(const Eigen::MatrixX<long double> &A) {
    Eigen::JacobiSVD<Eigen::MatrixX<long double>> svd(A, Eigen::ComputeThinV);
    const auto &V = svd.matrixV();
    stability_ = svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size() - 1);
    return V.col(V.cols() - 1);
}

Eigen::VectorX<long double> ConicFitter::fitConic(const std::vector<PatchPoint> &patchPoints) {
    int numPoints = int(patchPoints.size());
    numUnknowns_ = 6 + numPoints;

    numEq_ = numPoints * 3; // 1 eq per coordinate + 2 per normal
    return solveLinSystem(initAEigen(patchPoints));
}

double ConicFitter::stability() {
    double minVal = 10.0 * std::max(settings_.middlePointWeight, settings_.middleNormalWeight);
    double maxVal = 1.0e9 * minVal;
    double logMin = std::log10(minVal);
    double logMax = std::log10(maxVal);

    double logValue = std::log10(stability_);

    double mappedValue = (logValue - logMin) / (logMax - logMin);
    return std::clamp(mappedValue, 0.0, 1.0);
}
