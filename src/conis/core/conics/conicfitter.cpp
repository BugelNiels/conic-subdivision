#include "conicfitter.hpp"

#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/SVD>
#include <iostream>

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
        // Per point, add 3 equations: one for the point itself and two for the normal (x and y)
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
    return svd.matrixV().rightCols<1>();
}

Eigen::VectorX<real_t> ConicFitter::fitConic(const std::vector<PatchPoint> &patchPoints) {
    const int numPoints = int(patchPoints.size());
    // 6 for the conic coefficients + however many normal scaling factors we need to find
    numUnknowns_ = 6 + numPoints;
    // 1 eq per coordinate + 2 per normal
    numEq_ = numPoints * 3;
    return solveLinSystem(initAEigen(patchPoints));
}

} // namespace conis::core