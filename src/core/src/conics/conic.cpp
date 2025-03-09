#include "conis/core/conics/conic.hpp"

#include <cmath>
#include <iostream>
#include <utility>

#include "conis/core/conics/conicfitter.hpp"

namespace conis::core {

Conic::Conic(const real_t a,
             const real_t b,
             const real_t c,
             const real_t d,
             const real_t e,
             const real_t f,
             real_t epsilon)
    : epsilon_(epsilon) {
    Q_ << a, b, d, b, c, e, d, e, f;
    valid_ = !Q_.isZero() && Q_.allFinite(); // No fully zero matrix

    for (int i = 0; i < Q_.size(); ++i) {
        if (std::isnan(Q_(i))) {
            valid_ = false;
        }
    }
}
Conic::Conic(const Matrix3DD& Q, const real_t epsilon) : epsilon_(epsilon), Q_(Q) {}

Vector2DD Conic::conicNormal(const Vector2DD &p, const Vector2DD &rd) const {
    Vector2DD normal = conicNormal(p);
    if (normal.dot(rd) < 0.0) {
        normal *= -1;
    }
    return normal;
}

Vector2DD Conic::conicNormal(const Vector2DD &p) const {
    if (!valid_) {
        return {0, 0};
    }
#ifdef EXTRA_CONIC_PRECISION
    real_t xn = fmal(Q_(0, 0), p.x(), fmal(Q_(0, 1), p.y(), Q_(0, 2)));
    real_t yn = fmal(Q_(1, 0), p.x(), fmal(Q_(1, 1), p.y(), Q_(1, 2)));
#else
    Vector3DD p3(p.x(), p.y(), 1);
    real_t xn = Q_.row(0).dot(p3);
    real_t yn = Q_.row(1).dot(p3);
#endif
    return {xn, yn};
}

bool Conic::sample(const Vector2DD &origin, const Vector2DD &direction, Vector2DD &point, Vector2DD &normal) const {
    if (!valid_) {
        return false;
    }
    real_t t;
    if (intersects(origin, direction, t)) {
        point = origin + t * direction;
        normal = conicNormal(point, direction);
        if (normal.x() == 0.0 && normal.y() == 0.0) {
            // Failed to get a proper normal
            return false;
        }
        return true;
    }
#if 0
    printConic();
    std::cout << "Line((" << ro.x() << "," << ro.y() << "),(" << ro.x() + rd.x() << "," << ro.y() +  rd.y() << "))";
#endif
    return false;
}

/*
 * Computes a*b-c*d with a maximum error <= 1.5 ulp
 *
 *  Claude-Pierre Jeannerod, Nicolas Louvet, and Jean-Michel Muller,
 *  "Further Analysis of Kahan's Algorithm for the Accurate Computation
 *  of 2x2 Determinants". Mathematics of Computation, Vol. 82, No. 284,
 *  Oct. 2013, pp. 2245-2264
 */
static real_t diffOfProducts(const real_t a, const real_t b, const real_t c, const real_t d) {
    const real_t w = d * c;
    const real_t e = fmal(-d, c, w);
    const real_t f = fmal(a, b, -w);
    return f + e;
}

bool Conic::intersects(const Vector2DD &ro, const Vector2DD &rd, real_t &t) const {
    const Vector3DD p(ro.x(), ro.y(), 1);
    const Vector3DD u(rd.x(), rd.y(), 0);
#ifdef EXTRA_CONIC_PRECISION
    // Technically, b = p.dot(Q_ * u) + u.dot(Q_ * p);
    // However, since Q_ is symmetric, p.dot(Q_ * u) = u.dot(Q_ * p)
    // As such, b = 2 * u.dot(Q_ * p)
    // Because of this, we can remove the 4 in the discriminant calculation ((2b)^2 - 4ac = b^2 - ac
    // And we can also remove the 2 from the "/2a" part of the quadratic formula:
    // -2b+-sqrt(disc)/2a = -b+-sqrt(disc)/a
    const real_t a = u.dot(Q_ * u);
    const real_t b = u.dot(Q_ * p);
    const real_t c = p.dot(Q_ * p);

    if (std::abs(a) <= epsilon_) {
        t = -c / (2 * b);
        if (std::isnan(t)) {
            return false;
        }
        return true;
    }
    const real_t determinant = diffOfProducts(b, b, a, c);
    if (determinant < 0.0) {
        return false;
    }
    const real_t root = std::sqrt(determinant);
    if (std::isnan(root)) {
        return false;
    }
    // If b is negative, then -b is positive, so `-b - root` will always be smaller than `-b + root`
    if (b < 0) {
        t = (-b - root) / a;
    } else {
        t = (-b + root) / a;
    }
    return true;
#else
    const real_t a = u.dot(Q_ * u);
    const real_t b = 2 * u.dot(Q_ * p);
    const real_t c = p.dot(Q_ * p);
    if (std::abs(a) <= epsilon_) {
        t = -c / b;
        if (std::isnan(t)) {
            return false;
        }
        return true;
    }
    const real_t determinant = b * b - 4 * a * c;
    if (determinant < 0.0)
        return false;
    real_t t1 = (-b + std::sqrt(determinant)) / (2 * a);
    real_t t2 = (-b - std::sqrt(determinant)) / (2 * a);
    t = std::abs(t1) < std::abs(t2) ? t1 : t2;
    return true;
#endif
}

void Conic::printConic() const {
    std::cout << "Conic:";
    const double A = Q_(0, 0);
    const double D = Q_(0, 1);
    const double E = Q_(1, 1);
    const double G = Q_(0, 2);
    const double B = Q_(1, 2);
    const double F = Q_(2, 2);

    // Print the conic formula in Geogebra-compatible format
    std::ostringstream oss;
    oss << A << "*x^2 + " << D << "*x*y + " << E << "*y^2 + " << G << "*x + " << B << "*y + " << F << " = 0";
    std::cout << oss.str() << std::endl;
}

} // namespace conis::core
