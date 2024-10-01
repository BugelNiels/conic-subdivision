#pragma once

#include "core/vector.hpp"

namespace conics::core {

class Settings;

class Conic {
public:
    Conic() {};

    Conic(const std::vector<PatchPoint> &patchPoints, const Settings &settings);

    bool sample(const Vector2DD &origin,
                const Vector2DD &direction,
                Vector2DD &point,
                Vector2DD &normal) const;

    bool intersects(const Vector2DD &ro, const Vector2DD &rd, real_t &t) const;

    [[nodiscard]] real_t getStability() const;

    void printConic() const;

    Matrix3DD getMatrix() { return Q_; }

    [[nodiscard]] Vector2DD conicNormal(const Vector2DD &p, const Vector2DD &rd) const;
    [[nodiscard]] Vector2DD conicNormal(const Vector2DD &p) const;

private:
    real_t epsilon_ = 0;
    Matrix3DD Q_;
    real_t stability_ = 0;

    Matrix3DD fitConic(const std::vector<PatchPoint> &patchPoints);
};

} // namespace conics::core