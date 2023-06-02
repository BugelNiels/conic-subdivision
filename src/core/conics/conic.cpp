#include "conic.hpp"

#include <cmath>

#include "conicfitter.hpp"
#include "conicfitter_unit.hpp"

#include "src/core/settings.hpp"

Matrix3DD coefsToMatrix(const QVector<double> &coefs) {
    double a, b, c, d, e, f;
    a = coefs[0];           // A
    b = coefs[2];           // D
    c = coefs[1];           // E
    d = coefs[3];           // G
    e = coefs[4];           // B
    f = coefs[5];           // F
    Matrix3DD matrix;
    matrix << a, b, d, b, c, e, d, e, f;
    return matrix;
}

Conic::Conic(const Settings &settings) : settings_(settings) { Q_.fill(0); }

Conic::Conic(const QVector<Vector2DD> &coords,
             const QVector<Vector2DD> &normals, const Settings &settings)
        : Conic(settings) {
    hasSolution_ = fitConic(coords, normals);
}

void normalizeCoefs(QVector<double> &coefs) {
    double sum = 0;
    for (double &coef: coefs) {
        sum += coef;
    }

    double fac = 1 / sum;
    for (double &coef: coefs) {
        coef *= fac;
    }
}

/**
 * @brief Quadric::fitQuadric Attempts to fit a quadric to the provided patch
 * based on the provided solve settings.
 * @param patch The patch to fit the quadric to.
 * @param settings The solve settings used to fit the patch.
 * @return True if a quadric was constructed successfully. False otherwise.
 */
bool Conic::fitConic(const QVector<Vector2DD> &coords,
                     const QVector<Vector2DD> &normals) {
    QVector<double> foundCoefs;
    if (settings_.normalizedSolve) {
        UnitConicFitter fitter;
        foundCoefs = fitter.fitConic(coords, normals, settings_);
        stability_ = fitter.stability();
    } else {
        ConicFitter fitter(settings_);
        foundCoefs = fitter.fitConic(coords, normals);
        stability_ = fitter.stability();
    }
    if (foundCoefs.isEmpty()) {
        return false;
    }
//    normalizeCoefs(foundCoefs);

    Q_ = coefsToMatrix(foundCoefs);
    return true;
}

Vector2DD Conic::conicNormal(const Vector2DD &p, const Vector2DD &rd) const {
    Vector3DD p3(p.x(), p.y(), 1);
    double xn = Q_.row(0).dot(p3);
    double yn = Q_.row(1).dot(p3);
    Vector2DD normal(xn, yn);
    if (normal.dot(rd) < 0) {
        normal *= -1;
    }
//    normal.normalize();
    return normal;
}

bool Conic::sample(const Vector2DD &ro, const Vector2DD &rd, Vector2DD &p,
                   Vector2DD &normal) const {
    if (!isValid()) {
        return false;
    }
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

bool Conic::intersects(const Vector2DD &ro, const Vector2DD &rd,
                       double &t) const {
    Vector3DD p(ro.x(), ro.y(), 1);
    Vector3DD u(rd.x(), rd.y(), 0);
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
    double disc = b * b - a * c;
    if (disc < 0.0) {
        return false;
    }

    double root = std::sqrt(disc);
    double t0 = (-b - root) / a;
    double t1 = (-b + root) / a;
    if (std::fabs(t0) < std::fabs(t1)) {
        t = t0;
    } else {
        t = t1;
    }
    return true;
}

void Conic::printConic() const {
    qDebug() << "Conic:";
    double A = Q_(0, 0);
    double D = 2.0 * Q_(0, 1);
    double E = Q_(1, 1);
    double G = 2.0 * Q_(0, 2);
    double B = 2.0 * Q_(1, 2);
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
