#include "test_helpers.hpp"


namespace conis::core::test {
PointNormalPairs straightLine(int numPoints, real_t a, real_t b) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    real_t startX = -numPoints / 2.0;
    real_t endX = numPoints / 2.0;
    real_t step = (endX - startX) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        real_t x = startX + i * step;
        real_t y = a * x + b;
        points.emplace_back(x, y);

        // Normal to a line y = ax + b is (-a, 1) normalized
        Vector2DD normal(-a, 1);
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}
PointNormalPairs circle(int numPoints, real_t x, real_t y, real_t radius) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    real_t angleStep = 2 * M_PI / numPoints;

    for (int i = 0; i < numPoints; ++i) {
        real_t angle = i * angleStep;

        // Calculate point on the circle
        real_t px = x + radius * std::cos(angle);
        real_t py = y + radius * std::sin(angle);
        points.emplace_back(px, py);

        // Normal is the direction pointing outwards from the center
        Vector2DD normal(std::cos(angle), std::sin(angle));
        normals.push_back(normal);
    }

    return {points, normals};
}

PointNormalPairs ellipse(int numPoints, real_t x, real_t y, real_t a, real_t b) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    real_t angleStep = 2 * M_PI / numPoints;

    for (int i = 0; i < numPoints; ++i) {
        real_t angle = i * angleStep;

        // Parametric ellipse equation
        real_t px = x + a * std::cos(angle);
        real_t py = y + b * std::sin(angle);
        points.emplace_back(px, py);

        // Normal is perpendicular to the tangent at (a*cos(theta), b*sin(theta))
        Vector2DD normal(b * std::cos(angle), a * std::sin(angle));
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}

// Hardcoded for y^2 - 4x^2 = 4
PointNormalPairs hyperbolaSingleBranch(int numPoints) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    // go from -4 to 4
    real_t startX = -4;
    real_t step = 8 / static_cast<real_t>(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        real_t px = startX + i * step; // x values spaced linearly
        // y^2 - 4x^2 = 4 -> y = +-sqrt(4 + 4x^2), we only take + to ensure we are on a single branch
        real_t py = std::sqrt(4 * (px * px) + 4); // Solve for y using hyperbola equation
        points.emplace_back(px, py);

        // Gradient for normal calculation
        real_t nx = -8 * px;   // df/dx -> -8x
        real_t ny = 2 * py;  // df/dy -> 2y
        Vector2DD normal(nx, ny);
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}

// Hardcoded for x^2 - y = 0
PointNormalPairs parabola(int numPoints) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    // go from -4 to 4
    real_t startX = -4;
    real_t step = 8 / static_cast<real_t>(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        real_t px = startX + i * step; // x values spaced linearly
        real_t py = px * px; // y = x^2
        points.emplace_back(px, py);

        // Gradient for normal calculation
        real_t nx = 2 * px;  // df/dx = 2x
        real_t ny = -1;      // df/dy = -1
        Vector2DD normal(nx, ny);
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}

}