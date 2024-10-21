#pragma once

#include "conis/core/vector.hpp"

namespace conis::core {

class Conic {
public:
    Conic() = default;


    // Coefficients of the equation:
    // a*x^2 + b*x*y + c*y^2 + d*x + e*y + f
    Conic(real_t a, real_t b, real_t c, real_t d, real_t e, real_t f, real_t epsilon);
    Conic(Matrix3DD Q, real_t epsilon);

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
    bool sample(const Vector2DD &origin, const Vector2DD &direction, Vector2DD &point, Vector2DD &normal) const;

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
    bool valid_ = false;
    real_t epsilon_ = 0;
    Matrix3DD Q_;
};

} // namespace conis::core