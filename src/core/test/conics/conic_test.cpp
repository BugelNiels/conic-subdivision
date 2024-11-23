#include <gtest/gtest.h>
#include "conis/core/conics/conic.hpp"
#include "conis/core/vector.hpp"

using namespace conis::core;

constexpr real_t eps = 1e-6;

bool pointSatisfiesConic(const Eigen::VectorX<real_t>& conic, const Vector2DD& pt, real_t tolerance = eps) {
    const real_t x = pt.x();
    const real_t y = pt.y();
    const real_t result = conic[0] * x * x + conic[1] * x * y + conic[2] * y * y + conic[3] * x + conic[4] * y + conic[5];
    return std::abs(result) < tolerance;
}

// a*x^2 + b*x*y + c*y^2 + d*x + e*y + f
Vector2DD referenceConicNormal(const Vector2DD& p, real_t a, real_t b, real_t c, real_t d, real_t e, real_t f) {
    real_t x = p.x();
    real_t y = p.y();
    real_t nx = 2 * a * x + b * y + d;
    real_t ny = b * x + 2 * c * y + e;
    Vector2DD normal(nx, ny);
    return normal.normalized();
}


// Tests: normal calculation of conic

TEST(ConicTest, TestConicNormalEmptyIsZeroVector) {
    const Conic conic;
    const Vector2DD expectedNormal(0, 0);
    const Vector2DD actualNormal = conic.conicNormal({0, 0});
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalStraightLineHorizontal) {
    // Line y = 2
    const Conic conic(0, 0, 0, 0, 1, -2, eps);
    const Vector2DD point(0, 2);
    const Vector2DD expectedNormal = referenceConicNormal(point, 0, 0, 0, 0, 1, -2);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalStraightLineVertical) {
    // Line x = 3
    const Conic conic(0, 0, 0, 1, 0, -3, eps);
    const Vector2DD point(3, 0);
    const Vector2DD expectedNormal = referenceConicNormal(point, 0, 0, 0, 1, 0, -3);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalStraightLineDiagonal) {
    // Line y = x (45 degrees)
    const Conic conic(0, 0, 0, -1, 1, 0, eps);
    const Vector2DD point(1, 1);
    const Vector2DD expectedNormal = referenceConicNormal(point, 0, 0, 0, -1, 1, 0);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalCircle) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD point(5, 0);
    const Vector2DD expectedNormal = referenceConicNormal(point, 1, 0, 1, 0, 0, -25);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalCircleAtAngle) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD point(5 / std::sqrt(2), 5 / std::sqrt(2));
    const Vector2DD expectedNormal = referenceConicNormal(point, 1, 0, 1, 0, 0, -25);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalEllipse) {
    // Ellipse: 4*x^2 + y^2 - 25 = 0
    const Conic conic(4, 0, 1, 0, 0, -25, eps);
    const Vector2DD point(2.5, 0);
    const Vector2DD expectedNormal = referenceConicNormal(point, 4, 0, 1, 0, 0, -25);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalEllipseAtAngle) {
    // Ellipse: 4*x^2 + y^2 - 25 = 0
    const Conic conic(4, 0, 1, 0, 0, -25, eps);
    const Vector2DD point(2, 3);
    const Vector2DD expectedNormal = referenceConicNormal(point, 4, 0, 1, 0, 0, -25);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalHyperbola) {
    // Hyperbola: x^2 - y^2 - 1 = 0
    const Conic conic(1, 0, -1, 0, 0, -1, eps);
    const Vector2DD point(std::sqrt(2), 1);
    const Vector2DD expectedNormal = referenceConicNormal(point, 1, 0, -1, 0, 0, -1);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalParabola) {
    // Parabola: -x^2 + 4y = 0
    const Conic conic(-1, 0, 0, 0, 4, 0, eps);
    const Vector2DD point(2, 1);
    const Vector2DD expectedNormal = referenceConicNormal(point, -1, 0, 0, 0, 4, 0);
    const Vector2DD actualNormal = conic.conicNormal(point);
    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}

TEST(ConicTest, TestConicNormalParabolaAtVertex) {
    // Parabola: -x^2 + 4y = 0
    const Conic conic(-1, 0, 0, 0, 4, 0, eps);
    const Vector2DD point(0, 0); // Vertex of the parabola
    const Vector2DD expectedNormal = referenceConicNormal(point, -1, 0, 0, 0, 4, 0);
    const Vector2DD actualNormal = conic.conicNormal(point);

    ASSERT_NEAR(expectedNormal.x(), actualNormal.normalized().x(), eps);
    ASSERT_NEAR(expectedNormal.y(), actualNormal.normalized().y(), eps);
}


// Tests: intersections of ray with conic


TEST(ConicTest, TestIntersectsLine) {
    // Line y = 2
    const Conic conic(0, 0, 0, 0, 1, -2, eps);
    const Vector2DD ro(0, 0);    // Ray origin
    const Vector2DD rd(0, 1);    // Ray direction (upward)
    real_t t;

    ASSERT_TRUE(conic.intersects(ro, rd, t));
    ASSERT_NEAR(t, 2.0, eps); // Intersection should occur at y = 2
}

TEST(ConicTest, TestIntersectsCircle) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD ro(0, -10);  // Ray origin below the circle
    const Vector2DD rd(0, 1);    // Ray direction (upward)
    real_t t;

    ASSERT_TRUE(conic.intersects(ro, rd, t));
    ASSERT_NEAR(t, 5.0, eps); // Intersection should occur when y = -5
}

TEST(ConicTest, TestIntersectsCircleNoHit) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD ro(0, -10);  // Ray origin below the circle
    const Vector2DD rd(1, 0);    // Ray direction (horizontal, misses the circle)
    real_t t;

    ASSERT_FALSE(conic.intersects(ro, rd, t));
}

TEST(ConicTest, TestIntersectsEllipse) {
    // Ellipse: 4*x^2 + y^2 - 25 = 0
    const Conic conic(4, 0, 1, 0, 0, -25, eps);
    const Vector2DD ro(0, -10);  // Ray origin below the ellipse
    const Vector2DD rd(0, 1);    // Ray direction (upward)
    real_t t;

    ASSERT_TRUE(conic.intersects(ro, rd, t));
    ASSERT_NEAR(t, 5.0, eps); // Intersection at y = -5
}

TEST(ConicTest, TestIntersectsParabola) {
    // Parabola: -x^2 + 4y = 0
    const Conic conic(-1, 0, 0, 0, 4, 0, eps);
    const Vector2DD ro(4, 0);    // Ray origin to the right of the vertex
    const Vector2DD rd(0, 1);    // Ray direction (upward)
    real_t t;

    ASSERT_TRUE(conic.intersects(ro, rd, t));
    ASSERT_NEAR(t, 4, eps); // Expected intersection at y = 4
}

TEST(ConicTest, TestIntersectsEdgeCaseTangent) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD ro(0, 5);    // Ray origin exactly at the edge of the circle
    const Vector2DD rd(1, 0);    // Ray direction tangent to the circle (horizontal)
    real_t t;

    ASSERT_TRUE(conic.intersects(ro, rd, t)); // No intersection for tangent direction
    ASSERT_NEAR(t, 0, eps); // Expected intersection at the origin point
}

// Tests: Sampling of conic with ray
TEST(ConicTest, TestSampleLine) {
    // Line y = 2
    const Conic conic(0, 0, 0, 0, 1, -2, eps);
    const Vector2DD origin(0, 0);
    const Vector2DD direction(0, 1);  // Moving upward

    Vector2DD point, normal;
    ASSERT_TRUE(conic.sample(origin, direction, point, normal));
    ASSERT_NEAR(point.x(), 0, eps);
    ASSERT_NEAR(point.y(), 2, eps);  // Intersection at y = 2

    const Vector2DD expectedNormal = Vector2DD(0, 1);
    ASSERT_NEAR(normal.normalized().x(), expectedNormal.normalized().x(), eps);
    ASSERT_NEAR(normal.normalized().y(), expectedNormal.normalized().y(), eps);
}

TEST(ConicTest, TestSampleCircle) {
    // Circle with radius 5 centered at origin
    const Conic conic(1, 0, 1, 0, 0, -25, eps);
    const Vector2DD origin(0, -10);
    const Vector2DD direction(0, 1);  // Moving upward

    Vector2DD point, normal;
    ASSERT_TRUE(conic.sample(origin, direction, point, normal));
    ASSERT_NEAR(point.x(), 0, eps);
    ASSERT_NEAR(point.y(), -5, eps);  // Intersection at y = -5

    // Technically normal is (0, -1) but normal is flipped in same direction as the ray direction
    const Vector2DD expectedNormal = Vector2DD(0, 1); // Normal at this point
    ASSERT_NEAR(normal.normalized().x(), expectedNormal.normalized().x(), eps);
    ASSERT_NEAR(normal.normalized().y(), expectedNormal.normalized().y(), eps);
}

TEST(ConicTest, TestSampleEllipse) {
    // Ellipse: 4*x^2 + y^2 - 25 = 0
    const Conic conic(4, 0, 1, 0, 0, -25, eps);
    const Vector2DD origin(0, -10);
    const Vector2DD direction(0, 1);  // Moving upward

    Vector2DD point, normal;
    ASSERT_TRUE(conic.sample(origin, direction, point, normal));
    ASSERT_NEAR(point.x(), 0, eps);
    ASSERT_NEAR(point.y(), -5, eps);  // Intersection at y = -5

    const Vector2DD expectedNormal = Vector2DD(0, 1); // Normal at this point on the ellipse
    ASSERT_NEAR(normal.normalized().x(), expectedNormal.normalized().x(), eps);
    ASSERT_NEAR(normal.normalized().y(), expectedNormal.normalized().y(), eps);
}

TEST(ConicTest, TestSampleParabola) {
    // Parabola: -x^2 + 4y = 0
    const Conic conic(-1, 0, 0, 0, 4, 0, eps);
    const Vector2DD origin(2, 0);
    const Vector2DD direction(0, 1);  // Moving upward

    Vector2DD point, normal;
    ASSERT_TRUE(conic.sample(origin, direction, point, normal));
    ASSERT_NEAR(point.x(), 2, eps);
    ASSERT_NEAR(point.y(), 1, eps);  // Intersection at (2, 1)

    const Vector2DD expectedNormal = referenceConicNormal(point, -1, 0, 0, 0, 4, 0);
    ASSERT_NEAR(normal.normalized().x(), expectedNormal.normalized().x(), eps);
    ASSERT_NEAR(normal.normalized().y(), expectedNormal.normalized().y(), eps);
}
