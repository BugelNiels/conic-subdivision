#include "conic.hpp"

#include <cmath>
#include <iostream>
#include <QDebug>

#include "core/conics/conicfitter.hpp"

namespace conics::core {

static Matrix3DD coefsToMatrix(const Eigen::VectorX<real_t> &coefs) {
    real_t a = coefs[0]; // A - x*x
    real_t b = coefs[2]; // C - x*y
    real_t c = coefs[1]; // B - y*y
    real_t d = coefs[3]; // D - x
    real_t e = coefs[4]; // E - y
    real_t f = coefs[5]; // F - constant
    Matrix3DD matrix;
    matrix << a, b, d, b, c, e, d, e, f;
    return matrix;
}

Conic::Conic(const std::vector<PatchPoint> &patchPoints, real_t epsilon)
    : epsilon_(epsilon) {
    Q_ = fitConic(patchPoints);
}

/**
 * @brief Quadric::fitQuadric Attempts to fit a quadric to the provided patch
 * based on the provided solve settings.
 * @param patch The patch to fit the quadric to.
 * @return True if a quadric was constructed successfully. False otherwise.
 */
Matrix3DD Conic::fitConic(const std::vector<PatchPoint> &patchPoints) {
    ConicFitter fitter;
    Eigen::VectorX<real_t> foundCoefs = fitter.fitConic(patchPoints);
    stability_ = fitter.stability();
    return coefsToMatrix(foundCoefs);
}

Vector2DD Conic::conicNormal(const Vector2DD &p, const Vector2DD &rd) const {
    Vector2DD normal = conicNormal(p);
    if (normal.dot(rd) < 0.0) {
        normal *= -1;
    }
    return normal;
}

Vector2DD Conic::conicNormal(const Vector2DD &p) const {
    Vector3DD p3(p.x(), p.y(), 1);
    real_t xn = Q_.row(0).dot(p3);
    real_t yn = Q_.row(1).dot(p3);
    Vector2DD normal(xn, yn);
    return normal;
}

bool Conic::sample(const Vector2DD &ro,
                   const Vector2DD &rd,
                   Vector2DD &p,
                   Vector2DD &normal) const {
    real_t t;
    if (intersects(ro, rd, t)) {
        p = ro + t * rd;
        normal = conicNormal(p, rd);
        return true;
    }
#if 0
    printConic();
    qDebug() << "Line((" << ro.x() << "," << ro.y() << "),(" << ro.x() + rd.x() << "," << ro.y() +  rd.y() << "))";
#endif
    return false;
}

/*
  diff_of_products() computes a*b-c*d with a maximum error <= 1.5 ulp

  Claude-Pierre Jeannerod, Nicolas Louvet, and Jean-Michel Muller,
  "Further Analysis of Kahan's Algorithm for the Accurate Computation
  of 2x2 Determinants". Mathematics of Computation, Vol. 82, No. 284,
  Oct. 2013, pp. 2245-2264
*/
static real_t diff_of_products(real_t a, real_t b, real_t c, real_t d) {
    real_t w = d * c;
    real_t e = fmal(-d, c, w);
    real_t f = fmal(a, b, -w);
    return f + e;
}

bool Conic::intersects(const Vector2DD &ro, const Vector2DD &rd, real_t &t) const {
    Vector3DD p(ro.x(), ro.y(), 1);
    Vector3DD u(rd.x(), rd.y(), 0);
    // Technically, b = p.dot(Q_ * u) + u.dot(Q_ * p);
    // However, since Q_ is symmetric, p.dot(Q_ * u) = u.dot(Q_ * p)
    // As such, b = 2 * u.dot(Q_ * p)
    // Because of this, we can remove the 4 in the discriminant calculation ((2b)^2 - 4ac = b^2 - ac
    // And we can also remove the 2 from the "/2a" part of the quadratic formula:
    // -2b+-sqrt(disc)/2a = -b+-sqrt(disc)/a
    real_t a = u.dot(Q_ * u);
    real_t b = u.dot(Q_ * p);
    real_t c = p.dot(Q_ * p);

    if (std::fabs(a) < epsilon_) {
        t = -c / b;
        if (std::isnan(t)) {
            return false;
        }
        return true;
    }
    real_t disc = diff_of_products(b, b, a, c);
    if (disc < 0.0) {
        return false;
    }
    real_t root = std::sqrt(disc);

    real_t t0 = (-b - root) / a;
    real_t t1 = (-b + root) / a;
#if 1
    if (std::fabs(t0) < std::fabs(t1)) {
        t = t0;
    } else {
        t = t1;
    }
    return true;
#else
    if (t0 > 0 && t1 > 0) {
        t = std::min(t0, t1);
        return true;
    }
    if (t0 > 0) {
        t = t0;
        return true;
    }
    if (t1 > 0) {
        t = t1;
        return true;
    }
    return false;
#endif
}

void Conic::printConic() const {
    qDebug() << "Conic:";
    double A = Q_(0, 0);
    double D = Q_(0, 1);
    double E = Q_(1, 1);
    double G = Q_(0, 2);
    double B = Q_(1, 2);
    double F = Q_(2, 2);

    // Print the conic formula in Geogebra-compatible format
    qDebug() << QString("%1*x^2 + %2*x*y + %3*y^2 + %4*x + %5*y + %6 = 0")
                        .arg(A)
                        .arg(D)
                        .arg(E)
                        .arg(G)
                        .arg(B)
                        .arg(F);
}

real_t Conic::getStability() const {
    return stability_;
}

}