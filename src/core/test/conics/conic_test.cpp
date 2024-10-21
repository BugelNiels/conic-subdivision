#include <gtest/gtest.h>
#include "conis/core/conics/conic.hpp"
#include "conis/core/vector.hpp"

using namespace conis::core;

constexpr real_t eps = 1e-6;

bool pointSatisfiesConic(const Eigen::VectorX<real_t>& conic, const Vector2DD& pt, real_t tolerance = 1e-9) {
    const real_t x = pt.x();
    const real_t y = pt.y();
    const real_t result = conic[0] * x * x + conic[1] * x * y + conic[2] * y * y + conic[3] * x + conic[4] * y + conic[5];
    return std::abs(result) < tolerance;
}

void assertVectorsEqual(const Vector2DD& pt, const Vector2DD& pt2, const real_t tolerance = 1e-9) {
    ASSERT_EQ(pt.size(), pt2.size());
    for (size_t i = 0; i < pt.size(); ++i) {
        ASSERT_NEAR(pt[i], pt2[i], tolerance);
    }
}

// Tests: normal calculation of conic

TEST(ConicTest, TestConicNormalEmptyIsZeroVector) {
    const Conic conic;
    const Vector2DD expectedNormal(0,0);
    const Vector2DD actualNormal = conic.conicNormal({0,0});
    ASSERT_EQ(expectedNormal, actualNormal);
}

TEST(ConicTest, TestConicNormalStraightLine) {

}

TEST(ConicTest, TestConicNormalCircle) {

}

TEST(ConicTest, TestConicNormalEllipse) {

}

TEST(ConicTest, TestConicNormalHyperbola) {

}

TEST(ConicTest, TestConicNormalParabola) {

}

// Tests: intersections of ray with conic

TEST(ConicTest, TestIntersectEmpty) {

}

TEST(ConicTest, TestIntersectStraightLine) {

}

TEST(ConicTest, TestIntersectCircle) {

}

TEST(ConicTest, TestIntersectEllipse) {

}

TEST(ConicTest, TestIntersectHyperbola) {

}

TEST(ConicTest, TestIntersectParabola) {

}

// Tests: Sampling of conic with ray

TEST(ConicTest, TestSampleEmpty) {

}

TEST(ConicTest, TestSampleStraightLine) {

}

TEST(ConicTest, TestSampleCircle) {

}

TEST(ConicTest, TestSampleEllipse) {

}

TEST(ConicTest, TestSampleHyperbola) {

}

TEST(ConicTest, TestSampleParabola) {

}