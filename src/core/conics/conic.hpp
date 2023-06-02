#pragma once

#include <QMatrix4x4>
#include <QVector>
#include "util/vector.hpp"

class Settings;

class Conic {
public:
    explicit Conic(const Settings &settings);

    Conic(const QVector<Vector2DD> &coords, const QVector<Vector2DD> &normals,
          const Settings &settings);

    bool sample(const Vector2DD &origin, const Vector2DD &direction,
                Vector2DD &point, Vector2DD &normal) const;

    bool intersects(const Vector2DD &ro, const Vector2DD &rd, double &t) const;

    inline bool isValid() const { return hasSolution_; }

    inline Matrix4DD getCoefficients() const { return Q_; }

    Conic average(const Conic &other) const;

    Conic operator+(const Conic &other) const;

    void operator+=(const Conic &other);

    double getStability() const;

private:
    double stability_ = 0;
    Matrix4DD Q_;
    bool hasSolution_ = false;
    const Settings &settings_;

    bool fitConic(const QVector<Vector2DD> &coords,
                  const QVector<Vector2DD> &normals);

    Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;

    void printConic() const;
};
