#pragma once

#include "util/vector.hpp"

class Settings;

class Conic {
public:
    Conic(const std::vector<Vector2DD> &coords, const std::vector<Vector2DD> &normals,
          const Settings &settings);

    bool sample(const Vector2DD &origin, const Vector2DD &direction,
                Vector2DD &point, Vector2DD &normal) const;

    bool intersects(const Vector2DD &ro, const Vector2DD &rd, double &t) const;

    double getStability() const;

private:
    const Settings &settings_;
    Matrix3DD Q_;
    double stability_ = 0;

    Matrix3DD fitConic(const std::vector<Vector2DD> &coords,
                       const std::vector<Vector2DD> &normals);

    Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;

    void printConic() const;
};
