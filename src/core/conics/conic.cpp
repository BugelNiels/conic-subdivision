#include "conic.hpp"

#include <cmath>

#include "conicfitter.hpp"
#include "conicfitter_unit.hpp"

#include "src/core/settings.hpp"

#define EPSILON 0.0000000001

Matrix4DD coefsToMatrix(const QVector<double>& coefs) {
    double a, b, c, d, e, f;
    a = coefs[0];        // A
    b = coefs[2] / 2.0;  // D
    c = coefs[1];        // E
    d = coefs[3] / 2.0;  // G
    e = coefs[4] / 2.0;  // B
    f = coefs[5];        // F
    return Matrix4DD(a, b, d, 0, b, c, e, 0, d, e, f, 0, 0, 0, 0, 0);
}

Conic::Conic(const Settings &settings) : settings_(settings) { Q_.fill(0); }

Conic::Conic(const QVector<Vector2DD> &coords,
             const QVector<Vector2DD> &normals, const Settings &settings)
        : Conic(settings) {
    hasSolution_ = fitConic(coords, normals);
}

void normalizeCoefs(QVector<double> &coefs) {
    double fac = 1 / coefs[0];
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
//      normalizeCoefs(foundCoefs);

    Q_ = coefsToMatrix(foundCoefs);
    return true;
}

Vector2DD Conic::conicNormal(const Vector2DD &p, const Vector2DD &rd) const {
    Vector4DD p4(p.x(), p.y(), 1, 0);
    double xn = Q_.row(0).dot(p4);
    double yn = Q_.row(1).dot(p4);
    Vector2DD normal(xn, yn);
    normal.normalize();
    if (normal.dot(rd) < 0) {
        normal *= -1;
    }
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
    Vector4DD p(ro.x(), ro.y(), 1, 0);
    Vector4DD u(rd.x(), rd.y(), 0, 0);
    double a = u.dot(Q_ * u);
    double b = u.dot(Q_ * p);
    double c = p.dot(Q_ * p);
    if (std::fabs(a) < EPSILON) {
        t = -c / b;
        if(std::isnan(t)) {
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
    if (fabs(t0) < fabs(t1)) {
        t = t0;
    } else {
        t = t1;
    }
    return true;
}

Conic Conic::average(const Conic &other) const {
    Conic q = *this + other;
    q.Q_ /= 2.0;
    return q;
}

Conic Conic::operator+(const Conic &other) const {
    Conic q(settings_);
    q.hasSolution_ = true;
    if (isValid() && other.isValid()) {
        q.Q_ = Q_ + other.Q_;
    } else if (other.isValid()) {
        q.Q_ = other.Q_;
    } else if (isValid()) {
        q.Q_ = Q_;
    } else {
        q.hasSolution_ = false;
    }
    return q;
}

void Conic::operator+=(const Conic &other) {
    if (isValid() && other.isValid()) {
        Q_ = (Q_ + other.Q_);
        hasSolution_ = true;
    } else if (other.isValid()) {
        Q_ = other.Q_;
        hasSolution_ = true;
    }
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
