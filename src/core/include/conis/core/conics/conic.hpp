#pragma once

#include "conis/core/vector.hpp"

namespace conis::core {

class Conic {
public:
    Conic() = default;


    // The conic matrix Q represents the coefficients of the general conic equation:
    // ax^2 + bxy + cy^2 + dx + ey + f = 0.
    // It is constructed as a symmetric 3x3 matrix:
    //
    // Q = [ a    b/2   d/2 ]
    //     [ b/2  c     e/2 ]
    //     [ d/2  e/2   f   ]
    //
    // Note that the arguments here are therefore assumed to be:
    // ax^2 + 2bxy + cy^2 + 2dx + 2ey + f = 0.
    Conic(real_t a, real_t b, real_t c, real_t d, real_t e, real_t f, real_t epsilon);
    Conic(const Matrix3DD& Q, real_t epsilon);

    /**
     * @brief Checks whether the ray with the given origin and direction intersects and samples the intersection point.
     * If so, stores the intersection point and normal of the conic at said point in point and normal.
     *
     * @param origin Origin of the ray.
     * @param direction Direction of the ray.
     * @param point Location of the intersection point (if it hit).
     * @param normal Normal of the conic at the intersection point (if it hit).
     * @return true if the ray intersects
     * @return false otherwise
     */
    bool sample(const Vector2DD &origin, const Vector2DD &direction, Eigen::Ref<Vector2DD> point, Eigen::Ref<Vector2DD> normal) const;

    void printConic() const;

    const Matrix3DD& getMatrix() { return Q_; }

    /**
     * @brief Checks whether the given ray intersects this conic.
     *
     * @param ro Ray origin.
     * @param rd Ray direction.
     * @param t How far along the ray the intersection happened.
     * @return true if the ray intersects this conic.
     * @return false otherwise.
     */
    bool intersects(const Vector2DD &ro, const Vector2DD &rd, real_t &t) const;

    [[nodiscard]] Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;
    [[nodiscard]] Vector2DD conicNormal(const Vector2DD &p) const;

private:
    Matrix3DD Q_;
    bool valid_ = false;
    real_t epsilon_ = 0;
};

} // namespace conis::core
