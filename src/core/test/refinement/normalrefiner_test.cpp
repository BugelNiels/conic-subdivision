#include "conis/core/curve/curvepresetfactory.hpp"
#include "conis/core/curve/curvesaver.hpp"
#include "conis/core/refinement/normalrefiner.hpp"
#include "conis/core/subdivision/conicsubdivider.hpp"
#include "conis/core/subdivision/subdivisionsettings.hpp"
#include "conis/core/vector.hpp"
#include "test/test_helpers.hpp"
#include <gtest/gtest.h>

using namespace conis::core;

constexpr real_t eps = 1e-10;

class SmoothnessPenaltyTest : public ::testing::TestWithParam<CurvatureType> {};

INSTANTIATE_TEST_SUITE_P(CurvatureTypes,
                         SmoothnessPenaltyTest,
                         testing::Values(CurvatureType::CIRCLE_RADIUS,
                                         CurvatureType::AREA_INFLATION,
                                         CurvatureType::DISCRETE_WINDING,
                                         CurvatureType::GRADIENT_ARC_LENGTH));


//https://math.stackexchange.com/questions/1369155/how-do-i-rate-smoothness-of-discretely-sampled-data-picture

TEST_P(SmoothnessPenaltyTest, TestStraightLineEqual) {
    int numPoints = 6;
    auto [points, normals] = test::straightLine(numPoints, 0, 0);
    Curve curve(points, normals, true);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);
    for (int i = 0; i < numPoints; ++i) {
        real_t penalty = refiner.smoothnessPenalty(curve, i, GetParam());
        ASSERT_EQ(penalty, 0);
    }
}

TEST_P(SmoothnessPenaltyTest, TestCircleEqual) {
    int numPoints = 6;
    auto [points, normals] = test::circle(numPoints, 0, 0, 4.0);
    Curve curve(points, normals, true);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);
    for (int i = 0; i < numPoints; ++i) {
        real_t penaltyA = refiner.smoothnessPenalty(curve, i, GetParam());
        real_t penaltyB = refiner.smoothnessPenalty(curve, i + 1, GetParam());
        ASSERT_NEAR(penaltyA, penaltyB, eps);
    }
}

TEST_P(SmoothnessPenaltyTest, TestPositiveNegativeAnglesEqual) {
    /**
     * The curve we are testing is this:
     *
     *     2
     *    / \
     *   1   3
     *  /     \
     * 0       4
     *
     * And
     *
     * 0       4
     *  \     /
     *   1   3
     *    \ /
     *     2
     *
     * Where the smoothness penalty for the points 2 are expected to be equal.
     */
    std::vector<Vector2DD> pointsPositive{{0, 0}, {1, 2}, {2, 4}, {3, 2}, {4, 0}};
    std::vector<Vector2DD> pointsNegative{{0, 0}, {1, -2}, {2, -4}, {3, -2}, {4, -0}};
    Curve curvePositive(pointsPositive, false);
    Curve curveNegative(pointsNegative, false);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);
    real_t penaltyPositive = refiner.smoothnessPenalty(curvePositive, 2, GetParam());
    real_t penaltyNegative = refiner.smoothnessPenalty(curveNegative, 2, GetParam());
    ASSERT_NEAR(penaltyPositive, penaltyNegative, eps);
}

TEST_P(SmoothnessPenaltyTest, TestSharpVsFlat) {
    /**
    * Two curves where the y of one curve is multiplied by 2 to obtain a flatter and sharper curve respectively.
    *
     *     2
     *    / \
     *   1   3
     *  /     \
     * 0       4
     *
     * The sharper angle should result in a higer smoothness penalty.
     */
    std::vector<Vector2DD> pointsSharp{{0, 0}, {1, 4}, {2, 8}, {3, 4}, {4, 0}};
    std::vector<Vector2DD> pointsFlatter{{0, 0}, {1, 2}, {2, 4}, {3, 2}, {4, 0}};
    Curve curveSharp(pointsSharp, false);
    Curve curveFlatter(pointsFlatter, false);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);

    real_t penaltySharp = refiner.smoothnessPenalty(curveSharp, 2, GetParam());
    real_t penaltyFlatter = refiner.smoothnessPenalty(curveFlatter, 2, GetParam());
    ASSERT_LT(penaltyFlatter, penaltySharp);
}

TEST_P(SmoothnessPenaltyTest, TestConcaveVsConvex) {
    /**
    * A concave curve:
    *
     *   2
     *  / \
     * 1   3
     * |   |
     * 0   4
     *
     * And a convex curve:
     *
     *       2
     *      / \
     * 0 - 1   3 - 4
     *
     * Here the concave curve should have a lower smoothness penalty than the convex curve.
     */
    std::vector<Vector2DD> pointsConcave{{0, 0}, {0, 1}, {1, 2}, {2, 1}, {2, 0}};
    std::vector<Vector2DD> pointsConvex{{0, 0}, {1, 0}, {2, 1}, {3, 0}, {4, 0}};
    Curve curveConcave(pointsConvex, false);
    Curve curveConvex(pointsConcave, false);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);

    real_t penaltyConcave = refiner.smoothnessPenalty(curveConcave, 2, GetParam());
    real_t penaltyConvex = refiner.smoothnessPenalty(curveConvex, 2, GetParam());
    ASSERT_LT(penaltyConcave, penaltyConvex);
}

TEST_P(SmoothnessPenaltyTest, TestConcaveSharpness) {
    /**
    * Two concave curves are compared:
    *
     *   2
     *  / \
     * 1   3
     * |   |
     * 0   4
     *
     * And:
     *
     *   2
     *  / \
     * 1   3
     * |   |
     * |   |
     * |   |
     * 0   4
     *
     * The second curve is should have a higher smoothness penalty.
     */
    std::vector<Vector2DD> pointsConcaveA{{0, 0}, {0, 1}, {1, 2}, {2, 1}, {2, 0}};
    std::vector<Vector2DD> pointsConcaveB{{0, -2}, {0, 1}, {1, 2}, {2, 1}, {2, -2}};
    Curve curveConcaveA(pointsConcaveA, false);
    Curve curveConcaveB(pointsConcaveB, false);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);

    real_t penaltyConcaveA = refiner.smoothnessPenalty(curveConcaveA, 2, GetParam());
    real_t penaltyConcaveB = refiner.smoothnessPenalty(curveConcaveB, 2, GetParam());
    ASSERT_LT(penaltyConcaveA, penaltyConcaveB);
}

TEST_P(SmoothnessPenaltyTest, TestConvexSharpness) {
    /**
    * Two convex curves are compared:
    *
     *       2
     *      / \
     * 0 - 1   3 - 4
     *
     * And:
    *
     *         2
     *        / \
     * 0 - - 1   3 - - 4
     *
     * The second curve is should have a lower smoothness penalty.
     */
    std::vector<Vector2DD> pointsConvexA{{0, 0}, {1, 0}, {2, 1}, {3, 0}, {4, 0}};
    std::vector<Vector2DD> pointsConvexB{{-1, 0}, {1, 0}, {2, 1}, {3, 0}, {5, 0}};
    Curve curveConvexA(pointsConvexA, false);
    Curve curveConvexB(pointsConvexB, false);

    NormalRefinementSettings normRefSettings{};
    SubdivisionSettings subdivSettings{};

    NormalRefiner refiner(normRefSettings, subdivSettings);

    real_t penaltyConvexA = refiner.smoothnessPenalty(curveConvexA, 2, GetParam());
    real_t penaltyConvexB = refiner.smoothnessPenalty(curveConvexB, 2, GetParam());
    ASSERT_LT(penaltyConvexB, penaltyConvexA);
}