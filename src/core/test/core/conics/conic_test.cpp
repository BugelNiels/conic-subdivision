#include <gtest/gtest.h>
#include "conis/core/conics/conicfitter.hpp"
#include "conis/core/vector.hpp"

using namespace conis::core;

bool pointSatisfiesConic(const Eigen::VectorX<real_t>& conic, const Vector2DD& pt, real_t tolerance = 1e-9) {
    real_t x = pt.x();
    real_t y = pt.y();
    real_t result = conic[0] * x * x + conic[1] * x * y + conic[2] * y * y + conic[3] * x + conic[4] * y + conic[5];
    return std::abs(result) < tolerance;
}

// Tests: normal calculation of conic

TEST(ConicTest, TestConicNormalEmpty) {
    ConicFitter fitter;
}

TEST(ConicTest, TestConicNormalStraightLine) {
    ConicFitter fitter;
}

TEST(ConicTest, TestConicNormalCircle) {
    ConicFitter fitter;
}

TEST(ConicTest, TestConicNormalEllipse) {
    ConicFitter fitter;
}

TEST(ConicTest, TestConicNormalHyperbola) {
    ConicFitter fitter;
}

TEST(ConicTest, TestConicNormalParabola) {
    ConicFitter fitter;
}

// Tests: intersections of ray with conic

TEST(ConicTest, TestIntersectEmpty) {
    ConicFitter fitter;
}

TEST(ConicTest, TestIntersectStraightLine) {
    ConicFitter fitter;
}

TEST(ConicTest, TestIntersectCircle) {
    ConicFitter fitter;
}

TEST(ConicTest, TestIntersectEllipse) {
    ConicFitter fitter;
}

TEST(ConicTest, TestIntersectHyperbola) {
    ConicFitter fitter;
}

TEST(ConicTest, TestIntersectParabola) {
    ConicFitter fitter;
}

// Tests: Sampling of conic with ray

TEST(ConicTest, TestSampleEmpty) {
    ConicFitter fitter;
}

TEST(ConicTest, TestSampleStraightLine) {
    ConicFitter fitter;
}

TEST(ConicTest, TestSampleCircle) {
    ConicFitter fitter;
}

TEST(ConicTest, TestSampleEllipse) {
    ConicFitter fitter;
}

TEST(ConicTest, TestSampleHyperbola) {
    ConicFitter fitter;
}

TEST(ConicTest, TestSampleParabola) {
    ConicFitter fitter;
}