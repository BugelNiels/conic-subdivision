#pragma once

#include <memory>

#include "core/curve.hpp"
#include "core/subdivision/subdivider.hpp"
#include "util/vector.hpp"

class Settings;

/**
 * @brief The SubdivisionCurve class allows for easy subdivision of the provided using the provided subdivider.
 */
class SubdivisionCurve {
public:
    explicit SubdivisionCurve(const Settings &settings, Curve& controlCurve, std::shared_ptr<Subdivider> subdivider);

    [[nodiscard]] inline Curve& controlCurve() const { return controlCurve_; }

    [[nodiscard]] inline Curve& subdividedCurve() { return subdividedCurve_; }

    [[nodiscard]] inline int getSubdivLevel() const { return subdivisionLevel_; }

    void subdivide(int level);

    void reSubdivide();

private:
    const Settings &settings_;
    std::shared_ptr<Subdivider> subdivider_;

    Curve& controlCurve_;
    Curve subdividedCurve_;

    int subdivisionLevel_ = 0;
};
