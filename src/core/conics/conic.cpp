#include "conic.hpp"

#include <cmath>

#include "conicfitter.hpp"

#include "src/core/settings.hpp"

Matrix3DD coefsToMatrix(const Eigen::VectorXd &coefs) {
    double a, b, c, d, e, f;
    a = coefs[0];           // A - x*x
    b = coefs[2];           // C - x*y
    c = coefs[1];           // B - y*y
    d = coefs[3];           // D - x
    e = coefs[4];           // E - y
    f = coefs[5];           // F - constant
    Matrix3DD matrix;
    matrix << a, b, d, b, c, e, d, e, f;
    return matrix;
}

Conic::Conic(const QVector<Vector2DD> &coords,
             const QVector<Vector2DD> &normals, const Settings &settings)
        : settings_(settings) {
    Q_ = fitConic(coords, normals);
}

/**
 * @brief Quadric::fitQuadric Attempts to fit a quadric to the provided patch
 * based on the provided solve settings.
 * @param patch The patch to fit the quadric to.
 * @param settings The solve settings used to fit the patch.
 * @return True if a quadric was constructed successfully. False otherwise.
 */
Matrix3DD Conic::fitConic(const QVector<Vector2DD> &coords,
                          const QVector<Vector2DD> &normals) {
    ConicFitter fitter(settings_);
    Eigen::VectorXd foundCoefs = fitter.fitConic(coords, normals);
    stability_ = fitter.stability();
    return coefsToMatrix(foundCoefs);
}

Vector2DD Conic::conicNormal(const Vector2DD &p, const Vector2DD &rd) const {
    Vector3DD p3(p.x(), p.y(), 1);
    double xn = Q_.row(0).dot(p3);
    double yn = Q_.row(1).dot(p3);
    Vector2DD normal(xn, yn);
    if (normal.dot(rd) < 0.0) {
        normal *= -1;
    }
    return normal;
}

bool Conic::sample(const Vector2DD &ro, const Vector2DD &rd, Vector2DD &p,
                   Vector2DD &normal) const {
    double t;
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
double diff_of_products(double a, double b, double c, double d) {
    double w = d * c;
    double e = fma(-d, c, w);
    double f = fma(a, b, -w);
    return f + e;
}

bool Conic::intersects(const Vector2DD &ro, const Vector2DD &rd,
                       double &t) const {
    Vector3DD p(ro.x(), ro.y(), 1);
    Vector3DD u(rd.x(), rd.y(), 0);
    // Technically, b = p.dot(Q_ * u) + u.dot(Q_ * p);
    // However, since Q_ is symmetric, p.dot(Q_ * u) = u.dot(Q_ * p)
    // As such, b = 2 * u.dot(Q_ * p)
    // Because of this, we can remove the 4 in the discriminant calculation ((2b)^2 - 4ac = b^2 - ac
    // And we can also remove the 2 from the "/2a" part of the quadratic formula:
    // -2b+-sqrt(disc)/2a = -b+-sqrt(disc)/a

    double a = u.dot(Q_ * u);
    double b = u.dot(Q_ * p);
    double c = p.dot(Q_ * p);

    if (std::fabs(a) < settings_.epsilon) {
        t = -c / b;
        if (std::isnan(t)) {
            return false;
        }
        return true;
    }
    double disc = diff_of_products(b, b, a, c);
    if (disc < 0.0) {
        return false;
    }
    double root = std::sqrt(disc);
    
    double t0 = (-b - root) / a;
    double t1 = (-b + root) / a;
#if 1
    if (std::fabs(t0) < std::fabs(t1)) {
        t = t0;
    } else {
        t = t1;
    }
    return true;
#else
    if(t0 > 0) {
        t = t0;
        return true;
    }
    if(t1 > 0) {
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

double Conic::getStability() const {
    return stability_;
}
