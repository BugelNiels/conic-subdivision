#include "normalrefiner.hpp"
#include <iostream>

NormalRefiner::NormalRefiner(int maxIter) : maxIter(maxIter) {}

// Calculates the curvature at point b for the segment a-b-c
real_t calcCurvature(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) {
    Vector2DD ab = a - b;
    Vector2DD cb = c - b;
    real_t normAB = ab.norm();
    real_t normCB = cb.norm();
    real_t crossLength = std::abs(ab.x() * cb.y() - ab.y() * cb.x());
    return 2.0 * crossLength / (normAB * normCB * (normAB + normCB));
}

/**
 * @brief Calculates a number that judges the (local) smoothness of the curve. Lower numbers represent a smoother local segment.
 *
 * @param curve The curve to calculate the smoothness on
 * @param idx The index of the point on the curve for which to calculate the smoothness
 * @return int A number representing how smooth the curve is. Lower numbers are better (i.e. a smoother curve)
 */
int smoothnessPenalty(SubdivisionCurve &curve, int idx) {
    int n = curve.numPoints();

    const auto &points = curve.getCurveCoords();

    const auto &p_2 = points[(idx - 2 + n) % n];
    const auto &p_1 = points[(idx - 1 + n) % n];
    const auto &p0 = points[idx];
    const auto &p1 = points[(idx + 1) % n];
    const auto &p2 = points[(idx + 2) % n];

    real_t p_1Curv = calcCurvature(p_2, p_1, p0);
    real_t p0Curv =  calcCurvature(p_1, p0, p1);
    real_t p1Curv = calcCurvature(p0, p1, p2);

    std::cout << "p_1: " << p_1Curv << "  -  p0: " << p0Curv << "  -  p1: " << p1Curv << std::endl;

    // TODO: this is not a correct smoothness measure by a long shot
    real_t curvMeasure = std::fabs(p_1Curv + p1Curv - 2 * p0Curv);
    std::cout << curvMeasure << std::endl;
    std::cout.flush();
    return curvMeasure;
}

void binarySearchBestNormal(SubdivisionCurve &curve, Vector2DD &normal, int idx) {
    // To search the full unit, this should start at 0.5 (45 degrees)
    // This generally rotates too much and leads to difficult cases
    real_t angle = 0.1;

    // TODO: put this in settings
    real_t angleThreshold = 0.0001;

    // The subdivision level at which to test the smoothness
    // TODO: put this in settings
    int testSubdivLevel = 4;

    int n = curve.numPoints();
    Eigen::Matrix<real_t, 2, 2> rotationMatrix;

    // The idea is simple: we rotate both clockwise and counterclockwise by some angle
    // We pick whichever one results in a smoother curve
    // We half the angle and continue
    // Depending on what the starting angle is, this can search (part of) the unit circle efficiently
    while (angle > angleThreshold) {
        // Get the index of the net normal in the curve
        int normalIdxAtSubdivLevel = idx * std::pow(2, testSubdivLevel);

        // Clockwise setup
        real_t radians = angle * M_PI;
        rotationMatrix << std::cos(radians), std::sin(radians),
                         -std::sin(radians), std::cos(radians);
        Vector2DD clockwiseNormal = (rotationMatrix * normal).normalized();
        rotationMatrix.transposeInPlace();
        Vector2DD counterclockwiseNormal = (rotationMatrix * normal).normalized();

        // Calc curvature difference rotating clockwise
        normal = clockwiseNormal;
        curve.subdivide(testSubdivLevel);
        real_t clockwisePenalty = smoothnessPenalty(curve, normalIdxAtSubdivLevel);

        // Calc curvature difference rotating counterclockwise
        normal = counterclockwiseNormal;
        curve.subdivide(testSubdivLevel);
        real_t counterclockwisePenalty = smoothnessPenalty(curve, normalIdxAtSubdivLevel);

        // We pick whichever one results in the lower curvature penalty
        if (clockwisePenalty < counterclockwisePenalty) {
            normal = clockwiseNormal;
        } // else the normal is still on counterclockwise

        angle /= 2.0;
    }
}

void NormalRefiner::refine(SubdivisionCurve &curve) const {
    auto &norms = curve.getNetNormals();
    int n = norms.size();

    // TODO: do this until convergence with a maxIter being a max bound
    for (int i = 0; i < maxIter; i++) {
        for (int j = 0; j < n; j++) {
            binarySearchBestNormal(curve, norms[j], j);
        }
    }
}

void NormalRefiner::refineSelected(SubdivisionCurve &curve, int idx) const {
    auto &norms = curve.getNetNormals();
    int n = norms.size();
    binarySearchBestNormal(curve, norms[idx], idx);
}
