#include <gtest/gtest.h>
#include "conis/core/curve/curve.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/vector.hpp"

using namespace conis::core;

// Tests: normal calculation of conic


TEST(ConicSubdivisionTest, TestSubdivisionEmpty) {
    // We use the default subdivision settings. Also allows us to test if any change in the defaults break stuff
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
    ASSERT_EQ(curve.numPoints(), 0);
    ASSERT_EQ(curve.getVertices().size(), 0);
    ASSERT_EQ(curve.getNormals().size(), 0);
    ASSERT_EQ(curve.getCustomNormals().size(), 0);
}

TEST(ConicSubdivisionTest, TestNoSubdivisionReturnsSameCurve) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    Curve curve2 = curve;
    subdivider.subdivide(curve, 0);

}

TEST(ConicSubdivisionTest, TestNormalsPointInCorrectDirection) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    Curve curve2 = curve;
    subdivider.subdivide(curve, 0);
}

TEST(ConicSubdivisionTest, TestSubdivisionStraightLine) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
}

TEST(ConicSubdivisionTest, TestSubdivisionCircle) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
}

TEST(ConicSubdivisionTest, TestSubdivisionEllipse) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
}

TEST(ConicSubdivisionTest, TestSubdivisionHyperbola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
}

TEST(ConicSubdivisionTest, TestSubdivisionParabola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve;
    subdivider.subdivide(curve, 5);
}
