#include "conis/core/curve/curvesaver.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/curve/subdivision/subdivisionsettings.hpp"
#include "conis/core/vector.hpp"
#include "test/test_helpers.hpp"
#include <gtest/gtest.h>

using namespace conis::core;

constexpr real_t eps = 1e-1;
constexpr int testSubdivLevel = 12;

real_t gradientLength(const Vector2DD &p, real_t a, real_t b, real_t c, real_t d, real_t e, real_t f) {
    real_t x = p.x();
    real_t y = p.y();
    real_t nx = 2 * a * x + b * y + d;
    real_t ny = b * x + 2 * c * y + e;
    Vector2DD normal(nx, ny);
    return normal.norm();
}

//  a*x*x + b*x*y + c*y*y + d*x + e*y + f;
std::tuple<real_t, real_t> conicError(const std::vector<Vector2DD> &points,
                                      real_t a,
                                      real_t b,
                                      real_t c,
                                      real_t d,
                                      real_t e,
                                      real_t f) {
    real_t sum = 0;
    real_t max = 0;
    for (const auto &p: points) {
        real_t x = p.x();
        real_t y = p.y();
        real_t fp = a * x * x + b * x * y + c * y * y + d * x + e * y + f;
        real_t err = std::fabs(fp / gradientLength(p, a, b, c, d, e, f));
        max = std::max(err, max);
        sum += err;
    }
    return {sum / points.size(), max};
}

std::tuple<real_t, real_t> normalError(Curve &curve) {
    std::vector<Vector2DD> actualNormals = curve.getNormals();
    curve.recalculateNormals();
    const std::vector<Vector2DD> expectedNormals = curve.getNormals();

    // Only check the control normals.
    real_t sum = 0;
    real_t max = 0;
    int n = curve.numPoints();
    for (int i = 0; i < n; i++) {
        auto actualNormal = actualNormals[i].normalized();
        const auto expectedNormal = expectedNormals[i].normalized();
        // Make sure they point in the same direction for correct comparison
        if (actualNormal.dot(expectedNormal) < 0) {
            actualNormal *= -1;
        }

        // Clamp the dot product to avoid invalid acos inputs.
        const real_t dot = std::clamp(actualNormal.dot(expectedNormal),
                                      static_cast<real_t>(-1.0),
                                      static_cast<real_t>(1.0));
        const real_t angle = std::acos(dot);
        sum += angle;
        max = std::max(angle, max);
    }
    // Convert to degrees
    return {sum / n * (180.0 / M_PI), max * (180.0 / M_PI)};
}

TEST(ConicSubdivisionTest, TestSubdivisionEmpty) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);
    Curve curve({}, {}, true); // Empty curve

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
    for (int i = 0; i < 5; i++) {
        verts.emplace_back(i, 3 * i);
        normals.emplace_back(i, 3 * i);
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
    std::vector<Vector2DD> normals = {Vector2DD(-1, 1).normalized(),
                                      Vector2DD(-1, 1).normalized(),
                                      Vector2DD(-1, 1).normalized()};
    Curve curve(verts, normals, false);

    subdivider.subdivide(curve, 0);

    for (size_t i = 0; i < curve.getNormals().size(); ++i) {
        const Vector2DD &normal = curve.getNormals()[i];
        const Vector2DD &vertex = curve.getVertices()[i];
        ASSERT_NEAR(normal.dot(vertex.normalized()), 0, eps) << "Normal not perpendicular to vertex at index " << i;
    }
}

TEST(ConicSubdivisionTest, TestSubdivisionStraightLine) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    std::vector<Vector2DD> verts = {{0, 0}, {10, 0}};
    std::vector<Vector2DD> normals = {{0, 1}, {0, 1}};
    Curve curve(verts, normals, false);

    subdivider.subdivide(curve, 2);

    ASSERT_EQ(curve.getVertices().size(), 5); // 2 original points + 5 subdivisions
    ASSERT_EQ(curve.getNormals().size(), curve.getVertices().size());

    for (const auto &vertex: curve.getVertices()) {
        ASSERT_GE(vertex.x(), 0);
        ASSERT_LE(vertex.x(), 10);
        ASSERT_NEAR(vertex.y(), 0, eps);
    }

    for (const auto &normal: curve.getNormals()) {
        ASSERT_NEAR(normal.normalized().x(), 0, eps);
        ASSERT_NEAR(normal.normalized().y(), 1, eps);
    }
}

// x*x + y*y = 25
TEST(ConicSubdivisionTest, TestSubdivisionCircle) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    int numPoints = 6;
    int subdivLevel = testSubdivLevel;
    std::cout << "Subdiv level, mean conic error, max conic error, mean normal angle error, max normal angle error"
              << std::endl;
    // Note that we start from zero to ensure that the error is actually 0 at that point
    for (int i = 0; i <= subdivLevel; ++i) {
        auto [points, normals] = test::circle(numPoints, 0, 0, 5);
        Curve curve(points, normals, true);
        subdivider.subdivide(curve, i);

        ASSERT_EQ(curve.getVertices().size(), numPoints * std::pow(2, i)); // original points + 5 subdivisions
        ASSERT_EQ(curve.getNormals().size(), curve.getVertices().size());

        auto [meanError, maxError] = conicError(curve.getVertices(), 1, 0, 1, 0, 0, -25);
        auto [meanAngleError, maxAngleError] = normalError(curve);

        std::cout << i << ", " << meanError << ", " << maxError << ", " << meanAngleError << ", " << maxAngleError
                  << std::endl;
        ASSERT_LT(meanError, eps);
        ASSERT_LT(maxError, eps);
    }
}

// 4x*x + 9y*y = 36
TEST(ConicSubdivisionTest, TestSubdivisionEllipse) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    int numPoints = 6;
    int subdivLevel = testSubdivLevel;
    std::cout << "Subdiv level, mean conic error, max conic error, mean normal angle error, max normal angle error"
              << std::endl;
    // Note that we start from zero to ensure that the error is actually 0 at that point
    for (int i = 0; i <= subdivLevel; ++i) {
        auto [points, normals] = test::ellipse(numPoints, 0, 0, 3, 2);
        Curve curve(points, normals, true);
        subdivider.subdivide(curve, i);

        ASSERT_EQ(curve.getVertices().size(), numPoints * std::pow(2, i)); // original points + 5 subdivisions
        ASSERT_EQ(curve.getNormals().size(), curve.getVertices().size());

        auto [meanError, maxError] = conicError(curve.getVertices(), 4, 0, 9, 0, 0, -36);
        auto [meanAngleError, maxAngleError] = normalError(curve);

        std::cout << i << ", " << meanError << ", " << maxError << ", " << meanAngleError << ", " << maxAngleError
                  << std::endl;
        ASSERT_LT(meanError, eps);
        ASSERT_LT(maxError, eps);
    }
}

// y^2 - 4x^2 = 4
TEST(ConicSubdivisionTest, TestSubdivisionHyperbola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    int numPoints = 6;
    int subdivLevel = testSubdivLevel;
    std::cout << "Note that the normal error is slightly skewed here due to the end normals, which cannot be "
                 "accurately calculated"
              << std::endl;
    std::cout << "Subdiv level, mean conic error, max conic error, mean normal angle error, max normal angle error"
              << std::endl;
    // Note that we start from zero to ensure that the error is actually 0 at that point
    for (int i = 0; i <= subdivLevel; ++i) {
        auto [points, normals] = test::hyperbolaSingleBranch(numPoints);
        Curve curve(points, normals, false);

        CurveSaver saver;
        subdivider.subdivide(curve, i);

        ASSERT_EQ(curve.getVertices().size(), (numPoints - 1) * std::pow(2, i) + 1); // original points + 5 subdivisions
        ASSERT_EQ(curve.getNormals().size(), curve.getVertices().size());

        auto [meanError, maxError] = conicError(curve.getVertices(), -4, 0, 1, 0, 0, -4);
        auto [meanAngleError, maxAngleError] = normalError(curve);

        std::cout << i << ", " << meanError << ", " << maxError << ", " << meanAngleError << ", " << maxAngleError
                  << std::endl;
        ASSERT_LT(meanError, eps);
        ASSERT_LT(maxError, eps);
    }
}

// x^2 - y = 0
TEST(ConicSubdivisionTest, TestSubdivisionParabola) {
    SubdivisionSettings settings;
    ConicSubdivider subdivider(settings);

    int numPoints = 6;
    int subdivLevel = testSubdivLevel;
    std::cout << "Note that the normal error is slightly skewed here due to the end normals, which cannot be "
                 "accurately calculated"
              << std::endl;
    std::cout << "Subdiv level, mean conic error, max conic error, mean normal angle error, max normal angle error"
              << std::endl;
    // Note that we start from zero to ensure that the error is actually 0 at that point
    for (int i = 0; i <= subdivLevel; ++i) {
        auto [points, normals] = test::parabola(numPoints);
        Curve curve(points, normals, false);
        subdivider.subdivide(curve, i);

        ASSERT_EQ(curve.getVertices().size(), (numPoints - 1) * std::pow(2, i) + 1); // original points + 5 subdivisions
        ASSERT_EQ(curve.getNormals().size(), curve.getVertices().size());

        auto [meanError, maxError] = conicError(curve.getVertices(), 1, 0, 0, 0, -1, 0);
        auto [meanAngleError, maxAngleError] = normalError(curve);

        std::cout << i << ", " << meanError << ", " << maxError << ", " << meanAngleError << ", " << maxAngleError
                  << std::endl;
        ASSERT_LT(meanError, eps);
        ASSERT_LT(maxError, eps);
    }
}