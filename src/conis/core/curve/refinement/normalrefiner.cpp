#include "normalrefiner.hpp"

#include <iostream>

namespace conis::core {

NormalRefiner::NormalRefiner(const NormalRefinementSettings &normRefSettings, const SubdivisionSettings &subdivSettings)
    : normRefSettings_(normRefSettings),
      subdivider_(subdivSettings) {}

// Calculates the curvature at point b for the segment a-b-c
real_t NormalRefiner::calcCurvature(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) const {
    Vector2DD ab = a - b;
    Vector2DD cb = c - b;
    real_t normAB = ab.norm();
    real_t normCB = cb.norm();
    real_t crossLength = std::abs(ab.x() * cb.y() - ab.y() * cb.x());
    return 2.0 * crossLength / (normAB * normCB * (normAB + normCB));
}

real_t NormalRefiner::curvatureAtControlIdx(Curve &curve, int idx) const {
    int i = idx * std::pow(2, normRefSettings_.testSubdivLevel);
    int n = curve.numPoints();

    const auto &points = curve.getCoords();

    const auto &p_2 = points[(i - 2 + n) % n];
    const auto &p_1 = points[(i - 1 + n) % n];
    const auto &p0 = points[i];
    const auto &p1 = points[(i + 1) % n];
    const auto &p2 = points[(i + 2) % n];
    real_t c_1 = std::abs(calcCurvature(p_2, p_1, p0));
    real_t c0 = std::abs(calcCurvature(p_1, p0, p1));
    real_t c1 = std::abs(calcCurvature(p0, p1, p2));
    return std::abs(c_1 + c1 - 2 * c0);
}

/**
 * @brief Calculates a number that judges the (local) smoothness of the curve. Lower numbers represent a smoother local segment.
 *
 * @param curve The subdivided curve to calculate the smoothness on
 * @param idx The index of the point on the curve for which to calculate the smoothness
 * @param numControlPoints The number of points in the original control curve.
 * @return int A number representing how smooth the curve is. Lower numbers are better (i.e. a smoother curve)
 */
real_t NormalRefiner::smoothnessPenalty(Curve &curve, int idx, int numControlPoints) const {
    real_t penalty_1 = curvatureAtControlIdx(curve, (idx - 1 + numControlPoints) % numControlPoints);
    real_t penalty0 = curvatureAtControlIdx(curve, idx);
    real_t penalty1 = curvatureAtControlIdx(curve, (idx + 1) % numControlPoints);
    // This puts more weight on the middle one. Perhaps we can improve something here
    return penalty_1 + 2 * penalty0 + penalty1;
    // return penalty0;
}

void NormalRefiner::binarySearchBestNormal(Curve &curve, Vector2DD &normal, int idx) {
    // To search the full unit, this should start at 0.5 (45 degrees)
    // This generally rotates too much and leads to difficult cases
    const auto &controlPoints = curve.getCoords();
    int nc = curve.numPoints();
    // Find the normal of the line segment to the left
    Vector2DD ab = controlPoints[(idx - 1 + nc) % nc] - controlPoints[idx];
    ab = {-ab.y(), ab.x()};
    ab.normalize();
    // Find the normal of the line segment to the right
    Vector2DD cb = controlPoints[(idx + 1) % nc] - controlPoints[idx];
    cb = {cb.y(), -cb.x()};
    cb.normalize();
    // Put the normal halfway in between and set the search angle. This constrains the search
    normal = (ab + cb).normalized();
    // divide by 4, because half the angle is the angle between the normal (which is in the middle) and its two bounds
    // Since we are doing binary search, we need to half that again to ensure we don't go out of bounds
    real_t angle = acos(ab.dot(cb)) / 4.0;

    real_t angleThreshold = normRefSettings_.angleLimit;

    int n = curve.numPoints();
    Eigen::Matrix<real_t, 2, 2> rotationMatrix;

    Curve subdivCurve;

    // The idea is simple: we rotate both clockwise and counterclockwise by some angle
    // We pick whichever one results in a smoother curve
    // We half the angle and continue
    // Depending on what the starting angle is, this can search (part of) the unit circle efficiently
    while (angle > angleThreshold) {
        // Get the index of the net normal in the curve

        // Clockwise setup
        real_t radians = angle;
        rotationMatrix << std::cos(radians), std::sin(radians), -std::sin(radians), std::cos(radians);
        Vector2DD clockwiseNormal = (rotationMatrix * normal).normalized();
        rotationMatrix.transposeInPlace();
        Vector2DD counterclockwiseNormal = (rotationMatrix * normal).normalized();

        // Calc curvature difference rotating clockwise
        normal = clockwiseNormal;
        subdivCurve = curve;
        subdivider_.subdivide(subdivCurve, normRefSettings_.testSubdivLevel);
        real_t clockwisePenalty = smoothnessPenalty(subdivCurve, idx, nc);

        // Calc curvature difference rotating counterclockwise
        normal = counterclockwiseNormal;
        subdivCurve = curve;
        subdivider_.subdivide(subdivCurve, normRefSettings_.testSubdivLevel);
        real_t counterclockwisePenalty = smoothnessPenalty(subdivCurve, idx, nc);
        // We pick whichever one results in the lower curvature penalty
        if (clockwisePenalty < counterclockwisePenalty) {
            normal = clockwiseNormal;
        } // else the normal is still on counterclockwise

        angle /= 2.0;
    }
}

void NormalRefiner::refine(Curve &curve) {
    auto &norms = curve.getNormals();
    int n = norms.size();

    // TODO: do this until convergence with a maxIter being a max bound
    for (int i = 0; i < normRefSettings_.maxRefinementIterations; i++) {
        for (int j = 0; j < n; j++) {
            binarySearchBestNormal(curve, norms[j], j);
            std::cout << "Refined idx: " << j << std::endl;
        }
        std::cout << "Iteration done: " << i << std::endl;
    }
}

void NormalRefiner::refineSelected(Curve &curve, int idx) {
    auto &norms = curve.getNormals();
    int n = norms.size();
    binarySearchBestNormal(curve, norms[idx], idx);
}

} // namespace conis::core
