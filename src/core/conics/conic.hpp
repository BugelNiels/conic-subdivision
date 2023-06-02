#pragma once

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

    double getStability() const;

private:
    const Settings &settings_;
    Matrix3DD Q_;
    double stability_ = 0;
    bool hasSolution_ = false;

    bool fitConic(const QVector<Vector2DD> &coords,
                  const QVector<Vector2DD> &normals);

    Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;

    void printConic() const;
};
