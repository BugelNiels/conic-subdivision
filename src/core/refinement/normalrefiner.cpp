#include "normalrefiner.hpp"
#include <iostream>


NormalRefiner::NormalRefiner(int maxIter) : maxIter(maxIter) {

}


real_t calcCurvature(const Vector2DD& a, const Vector2DD&  b, const Vector2DD&  c) {
    Vector2DD ab = a - b;
    Vector2DD cb = c - b;

    real_t normAB = ab.norm();
    real_t normCB = cb.norm();


    Vector3DD ab3(ab.x(), ab.y(), 0);
    Vector3DD cb3(cb.x(), cb.y(), 0);
    // Norm of cross product
    real_t crossLength = std::abs(ab.x() * cb.y() - ab.y() * cb.x());
    // real_t crossLength = (ab.cross(cb));

    return 2.0 * crossLength / (normAB * normCB * (normAB + normCB));
}

int curvatureMeasure(SubdivisionCurve& curve, int idx) {
    // Central difference
    int n = curve.numPoints();

    const auto& points = curve.getCurveCoords();

    const auto& p_2 = points[(idx - 2 + n ) % n ];
    const auto& p_1 = points[(idx - 1 + n ) % n ];
    const auto& p0 = points[idx];
    const auto& p1 = points[(idx + 1) % n];
    const auto& p2 = points[(idx + 2) % n];

    real_t p_1Curv = calcCurvature(p_2, p_1, p0);
    // real_t p0Curv =  calcCurvature(p_1, p0, p1);
    real_t p1Curv =  calcCurvature(p0, p1, p2);

    // std::cout << "p_1: " << p_1Curv << "  -  p0: " << p0Curv << "  -  p1: " << p1Curv << std::endl;

    real_t curvMeasure = std::fabs(p_1Curv - p1Curv);
    std::cout << curvMeasure << std::endl;
    std::cout.flush();
    return curvMeasure;


}

void binaryAngleSearch(SubdivisionCurve& curve, Vector2DD& normal, int idx) {

    Eigen::Matrix<real_t, 2, 2> rotationMatrix;

    // We start by rotating 45 degrees, since more does not lead to desirable curves anyway
    real_t angle = 0.05;

    int testSubdivLevel = 4;
    int n = curve.numPoints();

    // TODO: until convergence
    for(int i = 0; i < 10; i++) {
        // Get the index of the net normal in the curve
        int normIdxAtSubdivLevel = idx * std::pow(2, testSubdivLevel);

        // Clockwise setup
        real_t radians = angle * M_PI;
        rotationMatrix << std::cos(radians), std::sin(radians),
                        -std::sin(radians), std::cos(radians);
        Vector2DD clockWiseNormal = (rotationMatrix * normal).normalized();
        rotationMatrix << std::cos(radians), -std::sin(radians),
                        std::sin(radians), std::cos(radians);
        Vector2DD antiClockWiseNormal = (rotationMatrix * normal).normalized();

        // Calc curvature difference rotating clockwise
        normal = clockWiseNormal;
        curve.subdivide(testSubdivLevel);
        real_t clockWiseCurv = curvatureMeasure(curve, normIdxAtSubdivLevel)
                                + curvatureMeasure(curve, (normIdxAtSubdivLevel - 1) % n);
                                + curvatureMeasure(curve, (normIdxAtSubdivLevel + 1) % n);

        // Calc curvature difference rotating anti clockwise
        normal = antiClockWiseNormal;
        curve.subdivide(testSubdivLevel);
        real_t antiClockWiseCurv = curvatureMeasure(curve, normIdxAtSubdivLevel)
                                + curvatureMeasure(curve, (normIdxAtSubdivLevel - 1) % n);
                                + curvatureMeasure(curve, (normIdxAtSubdivLevel + 1) % n);

        if(clockWiseCurv < antiClockWiseCurv) {
            normal = clockWiseNormal;
        }
        angle /= 2.0;
    }
}


void NormalRefiner::refine(SubdivisionCurve& curve) const {
    auto& norms = curve.getNetNormals();
    int n = norms.size();

    for(int i = 0; i < maxIter; i++) {
        for(int j = 0; j < n; j++) {
            binaryAngleSearch(curve, norms[j], j);
        }
    }
}

void NormalRefiner::refineSelected(SubdivisionCurve& curve, int idx) const {
    auto& norms = curve.getNetNormals();
    int n = norms.size();
    binaryAngleSearch(curve, norms[idx], idx);
}
