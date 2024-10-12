#include "normalrefiner.hpp"

#include <iostream>

namespace conis::core {

NormalRefiner::NormalRefiner(const NormalRefinementSettings &normRefSettings, const SubdivisionSettings &subdivSettings)
    : normRefSettings_(normRefSettings),
      subdivider_(subdivSettings) {}

real_t NormalRefiner::smoothnessPenalty(Curve &curve, int idx, CurvatureType curvatureType) const {
    int n = curve.numPoints();
    real_t curvature_1 = curve.curvatureAtIdx(curve.getPrevIdx(idx), curvatureType);
    real_t curvature1 = curve.curvatureAtIdx(curve.getNextIdx(idx), curvatureType);
    if (curvature1 > curvature_1) {
        return curvature1 / curvature_1;
    }
    return curvature_1 / curvature1;
    // return std::abs(curvature_1 - curvature1);
}

void NormalRefiner::binarySearchBestNormal(Curve &curve, int idx, bool inflectionPoint, CurvatureType curvatureType) {
    auto &normal = curve.getNormal(idx);
    real_t angle;
    if (inflectionPoint) {
        // In this case, restrict it between the edge and the normal of the edge it lies on
        // (an inflection point always lies in the middle of a straight edge)
        // Find the normal of the line segment to the left
        Vector2DD edge = curve.nextEdge(idx);
        Vector2DD edgeNormal = {edge.y(), -edge.x()};

        edgeNormal *= curve.edgePointingDir(idx);
        // Put the normal in the middle of the quadrant
        normal = (edge.normalized() + edgeNormal.normalized()).normalized();
        angle = M_PI / 8.0; // Search 45 degrees on either side
    } else {
        // Find the normal of the line segment to the left
        Vector2DD ab = curve.prevEdge(idx);
        ab = {ab.y(), -ab.x()};
        ab.normalize();
        // Find the normal of the line segment to the right
        Vector2DD cb = curve.nextEdge(idx);
        cb = {-cb.y(), cb.x()};
        cb.normalize();
        // Put the normal halfway in between and set the search angle. This constrains the search
        normal = (ab + cb).normalized();
        // divide by 4, because half the angle is the angle between the normal (which is in the middle) and its two bounds
        // Since we are doing binary search, we need to half that again to ensure we don't go out of bounds
        angle = acos(ab.dot(cb)) / 4.0;
    }

    Eigen::Matrix<real_t, 2, 2> rotationMatrix;
    // We need to calculate the smoothness at the given subdiv curve idx
    int testIdx = idx * std::pow(2, normRefSettings_.testSubdivLevel);

    // The idea is simple: we rotate both clockwise and counterclockwise by some angle
    // We pick whichever one results in a smoother curve
    // We half the angle and continue
    // Depending on what the starting angle is, this can search (part of) the unit circle efficiently
    while (angle > normRefSettings_.angleLimit) {
        // Set up the two normals we will test this iteration
        real_t radians = angle;
        rotationMatrix << std::cos(radians), std::sin(radians), -std::sin(radians), std::cos(radians);
        Vector2DD clockwiseNormal = (rotationMatrix * normal).normalized();
        rotationMatrix.transposeInPlace();
        Vector2DD counterclockwiseNormal = (rotationMatrix * normal).normalized();

        // Calc curvature difference rotating clockwise
        normal = clockwiseNormal;
        curve.copyDataTo(testCurve_);
        subdivider_.subdivide(testCurve_, normRefSettings_.testSubdivLevel);
        real_t clockwisePenalty = smoothnessPenalty(testCurve_, testIdx, curvatureType);

        // Calc curvature difference rotating counterclockwise
        normal = counterclockwiseNormal;
        curve.copyDataTo(testCurve_);
        subdivider_.subdivide(testCurve_, normRefSettings_.testSubdivLevel);
        real_t counterclockwisePenalty = smoothnessPenalty(testCurve_, testIdx, curvatureType);

        // We pick whichever one results in the lower curvature penalty
        if (clockwisePenalty < counterclockwisePenalty) {
            normal = clockwiseNormal;
        }
        angle /= 2.0;
    }
}

void NormalRefiner::refine(Curve &curve, CurvatureType curvatureType) {
    int n = curve.numPoints();
    Curve inflCurve = subdivider_.getInflPointCurve(curve);
    inflCurve.copyDataTo(curve);

    // Eventually do this until convergence with a maxIter as a max bound
    for (int i = 0; i < normRefSettings_.maxRefinementIterations; i++) {
        for (int j = 0; j < n; j++) {
            // The custom normal results correspond to the inflection points here
            binarySearchBestNormal(curve, j, curve.isCustomNormal(j), curvatureType);
            std::cout << "Refined idx: " << j << std::endl;
        }
        std::cout << "Iteration done: " << i << std::endl;
    }
}

void NormalRefiner::refineSelected(Curve &curve, CurvatureType curvatureType, int idx) {
    binarySearchBestNormal(curve, idx, false, curvatureType);
}

} // namespace conis::core
