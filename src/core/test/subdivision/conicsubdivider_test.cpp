#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/curve/subdivision/subdivisionsettings.hpp"
#include "conis/core/vector.hpp"
#include "test/test_helpers.hpp"
#include <gtest/gtest.h>

using namespace conis::core;

constexpr real_t eps = 1e-6;


TEST(ConicSubdivisionTest, TestSubdivisionEmpty) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve({}, {}, true);  // Empty curve

    subdivider.subdivide(curve, 5);

    ASSERT_EQ(curve.numPoints(), 0);
    ASSERT_EQ(curve.getVertices().size(), 0);
    ASSERT_EQ(curve.getNormals().size(), 0);
    ASSERT_EQ(curve.getCustomNormals().size(), 0);
}

TEST(ConicSubdivisionTest, TestNoSubdivisionReturnsSameCurve) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    std::vector<Vector2DD> verts, normals;
    for(int i =0; i < 5; i++) {
        verts.emplace_back(i, 3*i);
        normals.emplace_back(i, 3*i);
    }
    Curve curve(verts, normals, true);

    // Create a copy of the original curve for comparison
    Curve originalCurve = curve;
    subdivider.subdivide(curve, 0);

    ASSERT_EQ(curve.getVertices(), originalCurve.getVertices());
    ASSERT_EQ(curve.getNormals(), originalCurve.getNormals());
    ASSERT_EQ(curve.getCustomNormals(), originalCurve.getCustomNormals());
}

TEST(ConicSubdivisionTest, TestNormalsPointInCorrectDirection) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    std::vector<Vector2DD> verts = {{0, 0}, {1, 1}, {2, 2}};
    std::vector<Vector2DD> normals = {Vector2DD(-1, 1).normalized(), Vector2DD(-1, 1).normalized(), Vector2DD(-1, 1).normalized()};
    Curve curve(verts, normals, false);

    subdivider.subdivide(curve, 0);

    for (size_t i = 0; i < curve.getNormals().size(); ++i) {
        const Vector2DD& normal = curve.getNormals()[i];
        const Vector2DD& vertex = curve.getVertices()[i];
        ASSERT_NEAR(normal.dot(vertex.normalized()), 0, eps) << "Normal not perpendicular to vertex at index " << i;
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionStraightLine) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    std::vector<Vector2DD> verts = {{0, 0}, {10, 0}};
    std::vector<Vector2DD> normals = {{0, 1}, {0, 1}};
    Curve curve(verts, normals, false);

    subdivider.subdivide(curve, 3);

    ASSERT_EQ(curve.getVertices().size(), 6); // 2 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), 6);

    for (const auto& normal : curve.getNormals()) {
        ASSERT_NEAR(normal.x(), 0, eps);
        ASSERT_NEAR(normal.y(), 1, eps);
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionCircle) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    const double radius = 5.0;
    auto [points, normals] = test::circle(20, 0, 0, radius);
    Curve curve(points, normals, true);

    subdivider.subdivide(curve, 5);

    ASSERT_EQ(curve.getVertices().size(), 25); // 20 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), 25);

    for (size_t i = 0; i < curve.getVertices().size(); ++i) {
        Vector2DD vertex = curve.getVertices()[i];
        Vector2DD expectedNormal = vertex.normalized();
        ASSERT_NEAR(curve.getNormals()[i].x(), expectedNormal.x(), eps);
        ASSERT_NEAR(curve.getNormals()[i].y(), expectedNormal.y(), eps);
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionEllipse) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    auto [points, normals] = test::ellipse(20, 0, 0, 3, 2);
    Curve curve(points, normals, true);

    subdivider.subdivide(curve, 5);

    ASSERT_EQ(curve.getVertices().size(), 25); // 20 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), 25);

    for (size_t i = 0; i < curve.getVertices().size(); ++i) {
        Vector2DD vertex = curve.getVertices()[i];
        Vector2DD expectedNormal = vertex.normalized();
        ASSERT_NEAR(curve.getNormals()[i].dot(expectedNormal), 1, eps);
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionHyperbola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    auto [points, normals] = test::hyperbola(20, 0, 0, 2, 1);
    Curve curve(points, normals, false);

    subdivider.subdivide(curve, 5);

    ASSERT_EQ(curve.getVertices().size(), 25); // 20 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), 25);

    for (size_t i = 0; i < curve.getNormals().size(); ++i) {
        Vector2DD normal = curve.getNormals()[i];
        Vector2DD tangent = Vector2DD(-normal.y(), normal.x()); // Perpendicular vector
        ASSERT_NEAR(tangent.dot(curve.getVertices()[i].normalized()), 0, eps);
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionParabola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    auto [points, normals] = test::parabola(20, 1, 0, 0);
    Curve curve(points, normals, false);

    subdivider.subdivide(curve, 5);

    ASSERT_EQ(curve.getVertices().size(), 25); // 20 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), 25);

    for (size_t i = 0; i < curve.getVertices().size(); ++i) {
        Vector2DD vertex = curve.getVertices()[i];
        Vector2DD expectedNormal = vertex.normalized();
        ASSERT_NEAR(curve.getNormals()[i].dot(expectedNormal), 1, eps);
    }
}