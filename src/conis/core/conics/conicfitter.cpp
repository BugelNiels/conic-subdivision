#include "conicfitter.hpp"

#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/SVD>

namespace conis::core {

ConicFitter::ConicFitter() {}

static void pointEqEigen(Eigen::RowVectorX<real_t> &row, const Vector2DD &coord, int numUnknowns) {
    row.setZero();
    const real_t x = coord.x();
    const real_t y = coord.y();
    row(0) = x * x;
    row(1) = y * y;
    row(2) = 2 * x * y;
    row(3) = 2 * x;
    row(4) = 2 * y;
    row(5) = 1;
}

static void normEqXEigen(Eigen::RowVectorX<real_t> &row,
                         const Vector2DD &coord,
                         const Vector2DD &normal,
                         int numUnknowns,
                         int normIdx) {
    row.setZero();
    const real_t x = coord.x();
    const real_t y = coord.y();
    row(0) = 2 * x; // A
    // row(1) = 0;     // B
    row(2) = 2 * y; // C
    row(3) = 2;     // D
    // row(4) = 0;     // E
    // row(5) = 0;     // F
    row(6 + normIdx) = -normal.x();
}

static void normEqYEigen(Eigen::RowVectorX<real_t> &row,
                         const Vector2DD &coord,
                         const Vector2DD &normal,
                         int numUnknowns,
                         int normIdx) {
    row.setZero();
    const real_t x = coord.x();
    const real_t y = coord.y();
    // row(0) = 0;     // A
    row(1) = 2 * y; // B
    row(2) = 2 * x; // C
    // row(3) = 0;     // E
    row(4) = 2; // F
    // row(5) = 0;     // G
    row(6 + normIdx) = -normal.y();
}

Eigen::Matrix<real_t, Eigen::Dynamic, Eigen::Dynamic> ConicFitter::initAEigen(
        const std::vector<PatchPoint> &patchPoints) const {
    Eigen::MatrixX<real_t> A(numEq_, numUnknowns_);

    // Reuse this row buffer
    Eigen::RowVectorX<real_t> row = Eigen::RowVectorX<real_t>::Zero(numUnknowns_);

    int rowIdx = 0;
    for (int i = 0; i < patchPoints.size(); i++) {
        auto &p = patchPoints[i];
        const Vector2DD &coord = p.coords;
        const Vector2DD &normal = p.normal;
        pointEqEigen(row, coord, numUnknowns_);
        A.row(rowIdx++) = row * p.pointWeight;
        normEqXEigen(row, coord, normal, numUnknowns_, i);
        A.row(rowIdx++) = row * p.normWeight;
        normEqYEigen(row, coord, normal, numUnknowns_, i);
        A.row(rowIdx++) = row * p.normWeight;
    }
    return A;
}

Eigen::VectorX<real_t> ConicFitter::solveLinSystem(const Eigen::MatrixX<real_t> &A) {
    const Eigen::JacobiSVD<Eigen::MatrixX<real_t>> svd(A, Eigen::ComputeThinV);
    const auto &V = svd.matrixV();
    stability_ = svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size() - 1);
    return V.col(V.cols() - 1);
}

Eigen::VectorX<real_t> ConicFitter::fitConic(const std::vector<PatchPoint> &patchPoints) {
    const int numPoints = int(patchPoints.size());
    // 6 for the conic coefficients + however many normal scaling factors we need to find
    numUnknowns_ = 6 + numPoints;
    // 1 eq per coordinate + 2 per normal
    numEq_ = numPoints * 3;
    return solveLinSystem(initAEigen(patchPoints));
}

real_t ConicFitter::stability() const {
    const real_t minVal = 10.0 * 100000;
    const real_t maxVal = 1.0e9 * minVal;
    const real_t logMin = std::log10(minVal);
    const real_t logMax = std::log10(maxVal);
    const real_t logValue = std::log10(stability_);
    const real_t mappedValue = (logValue - logMin) / (logMax - logMin);
    return std::clamp(mappedValue, real_t(0), real_t(1));
}

} // namespace conis::core