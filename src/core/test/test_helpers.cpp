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

PointNormalPairs hyperbola(int numPoints, real_t x, real_t y, real_t a, real_t b) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    real_t step = 4 * a / (numPoints - 1); // spread points along one branch

    for (int i = 0; i < numPoints; ++i) {
        real_t t = -2 * a + i * step;

        // Parametric hyperbola equation for one branch (x = a sec(t), y = b tan(t))
        real_t px = x + t;
        real_t py = y + (b / a) * std::sqrt(t * t - a * a);
        points.emplace_back(px, py);

        // Normal to hyperbola at this point
        Vector2DD normal(b * t, -a * std::sqrt(t * t - a * a));
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}

PointNormalPairs parabola(int numPoints, real_t a, real_t b, real_t c) {
    std::vector<Vector2DD> points;
    std::vector<Vector2DD> normals;

    real_t startX = -numPoints / 2.0;
    real_t endX = numPoints / 2.0;
    real_t step = (endX - startX) / (numPoints - 1);

    for (int i = 0; i < numPoints; ++i) {
        real_t x = startX + i * step;
        real_t y = a * x * x + b * x + c;
        points.emplace_back(x, y);

        // Derivative of y = ax^2 + bx + c is dy/dx = 2ax + b
        // The normal vector is perpendicular to the tangent, so (-dy, dx)
        real_t dy_dx = 2 * a * x + b;
        Vector2DD normal(-dy_dx, 1);
        normal.normalize();
        normals.push_back(normal);
    }

    return {points, normals};
}
}