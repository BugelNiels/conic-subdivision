#include "normalrefiner.hpp"

#include <iostream>

// #define DEBUG_OUTPUT

namespace conis::core {

NormalRefiner::NormalRefiner(const NormalRefinementSettings &normRefSettings, const SubdivisionSettings &subdivSettings)
    : normRefSettings_(normRefSettings),
      subdivider_(subdivSettings) {}

// Calculates the curvature at point b for the segment a-b-c
real_t NormalRefiner::calcCurvature(const Vector2DD &a,
                                    const Vector2DD &b,
                                    const Vector2DD &c,
                                    CurvatureType curvatureType) const {
    if (curvatureType == CurvatureType::CIRCLE_RADIUS) {
        Vector2DD ab = a - b;
        Vector2DD cb = c - b;
        Vector2DD ac = a - c;

        real_t denom = ab.dot(ab) * cb.dot(cb) * ac.dot(ac);

        // Avoid division by zero
        if (denom == 0.0)
            return 0.0;
        real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
        return sqrt((cross * cross) / denom);
    }

    const auto e1 = b - a;
    const auto e_1 = c - a;
    real_t cross = e_1.x() * e1.y() - e_1.y() * e1.x();
    real_t dot = e_1.x() * e1.x() + e_1.y() * e1.y();
    real_t v = atan(cross / dot);

    real_t denom = e_1.norm() + e1.norm();
    if (denom == 0.0)
        return 0.0;

    if (curvatureType == CurvatureType::DISCRETE_WINDING) {
        return 2.0 * v / denom;
    } else if (curvatureType == CurvatureType::GRADIENT_ARC_LENGTH) {
        return 4.0 * sin(v / 2.0) / denom;
    } else if (curvatureType == CurvatureType::AREA_INFLATION) {
        return 4.0 * tan(v / 2.0) / denom;
    }
    std::cerr << "Unsupported curvature type: " << curvatureType;
    return 0; // Unsupported curvature type
}

real_t NormalRefiner::curvatureAtIdx(Curve &curve, int i, CurvatureType curvatureType) const {
    int n = curve.numPoints();
    const auto &points = curve.getCoords();
    const auto &p_1 = points[(i - 1 + n) % n];
    const auto &p0 = points[i];
    const auto &p1 = points[(i + 1) % n];

    return calcCurvature(p_1, p0, p1, curvatureType);
}

/**
 * @brief Calculates a number that judges the (local) smoothness of the curve. Lower numbers represent a smoother local segment.
 *
 * @param curve The subdivided curve to calculate the smoothness on
 * @param idx The index of the point on the curve for which to calculate the smoothness
 * @return int A number representing how smooth the curve is. Lower numbers are better (i.e. a smoother curve)
 */
real_t NormalRefiner::smoothnessPenalty(Curve &curve, int idx, CurvatureType curvatureType) const {
    int n = curve.numPoints();
    int i = idx * std::pow(2, normRefSettings_.testSubdivLevel);
    real_t curvature_1 = curvatureAtIdx(curve, (i - 1 + n) % n, curvatureType);
    real_t curvature1 = curvatureAtIdx(curve, (i + 1) % n, curvatureType);
    return std::abs(curvature_1 - curvature1);
}

void NormalRefiner::binarySearchBestNormal(Curve &curve, int idx, CurvatureType curvatureType) {
    // To search the full unit, this should start at 0.5 (45 degrees)
    // This generally rotates too much and leads to difficult cases
    const auto &controlPoints = curve.getCoords();
    auto &normal = curve.getNormals()[idx];
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
        curve.copyDataTo(testCurve_);
        subdivider_.subdivide(testCurve_, normRefSettings_.testSubdivLevel);
        real_t clockwisePenalty = smoothnessPenalty(testCurve_, idx, curvatureType);

        // Calc curvature difference rotating counterclockwise
        normal = counterclockwiseNormal;
        curve.copyDataTo(testCurve_);
        subdivider_.subdivide(testCurve_, normRefSettings_.testSubdivLevel);
        real_t counterclockwisePenalty = smoothnessPenalty(testCurve_, idx, curvatureType);
#ifdef DEBUG_OUTPUT
        std::cout << clockwisePenalty << " <c cc> " << counterclockwisePenalty << std::endl;
#endif
        // We pick whichever one results in the lower curvature penalty
        if (clockwisePenalty < counterclockwisePenalty) {
            normal = clockwiseNormal;
#ifdef DEBUG_OUTPUT
            std::cout << "\t clockwise" << std::endl;
#endif
        } else {
#ifdef DEBUG_OUTPUT
            std::cout << "\t counter clockwise" << std::endl; // else the normal is still on counterclockwise
#endif
        }
        angle /= 2.0;
    }
}

void NormalRefiner::refine(Curve &curve, CurvatureType curvatureType) {
    int n = curve.numPoints();

    // TODO: do this until convergence with a maxIter being a max bound
    for (int i = 0; i < normRefSettings_.maxRefinementIterations; i++) {
        for (int j = 0; j < n; j++) {
            binarySearchBestNormal(curve, j, curvatureType);
            std::cout << "Refined idx: " << j << std::endl;
        }
        std::cout << "Iteration done: " << i << std::endl;
    }
}

void NormalRefiner::refineSelected(Curve &curve, CurvatureType curvatureType, int idx) {
    binarySearchBestNormal(curve, idx, curvatureType);
}

} // namespace conis::core
