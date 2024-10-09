#pragma once

#include "core/curve/curve.hpp"
#include "core/curve/refinement/normalrefinementsettings.hpp"
#include "core/curve/subdivision/conicsubdivider.hpp"
#include "core/curve/subdivision/subdivisionsettings.hpp"
#include "core/vector.hpp"

namespace conis::core {

class NormalRefiner {
public:
    explicit NormalRefiner(const NormalRefinementSettings &normRefSettings, const SubdivisionSettings &subdivSettings);

    void refine(Curve &curve);
    void refineSelected(Curve &curve, int idx);

private:
    const NormalRefinementSettings &normRefSettings_;
    ConicSubdivider subdivider_;

    real_t calcCurvature(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c) const;

    real_t curvatureAtControlIdx(Curve &curve, int idx) const;

    real_t smoothnessPenalty(Curve &curve, int idx, int numControlPoints) const;

    void binarySearchBestNormal(Curve &curve, Vector2DD &normal, int idx);
};

} // namespace conis::core
