#pragma once

#include "util/vector.hpp"

class Settings;

class Conic {
public:
    Conic(const std::vector<PatchPoint> &patchPoints, const Settings &settings);

    bool sample(const Vector2DD &origin, const Vector2DD &direction,
                Vector2DD &point, Vector2DD &normal) const;

    bool intersects(const Vector2DD &ro, const Vector2DD &rd, long double &t) const;

    [[nodiscard]] double getStability() const;

private:
    const Settings &settings_;
    Matrix3DD Q_;
    double stability_ = 0;

    Matrix3DD fitConic(const std::vector<PatchPoint> &patchPoints);

    [[nodiscard]] Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;

    void printConic() const;
};
