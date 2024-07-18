#pragma once

#include "src/core/settings.hpp"
#include "src/core/subdivisioncurve.hpp"
#include "util/vector.hpp"

class NormalRefiner {
public:
    explicit NormalRefiner(const Settings &settings);

    void refine(SubdivisionCurve &curve) const;
    void refineSelected(SubdivisionCurve &curve, int idx) const;

private:
    const Settings &settings;

    real_t calcCurvature(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) const;

    real_t curvatureAtControlIdx(SubdivisionCurve &curve, int idx) const;

    real_t smoothnessPenalty(SubdivisionCurve &curve, int idx) const;

    void binarySearchBestNormal(SubdivisionCurve &curve, Vector2DD &normal, int idx) const;
};
