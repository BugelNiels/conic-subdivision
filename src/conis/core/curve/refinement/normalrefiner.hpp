#pragma once

#include "conis/core/curve/curve.hpp"
#include "conis/core/curve/refinement/curvaturetype.hpp"
#include "conis/core/curve/refinement/normalrefinementsettings.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/curve/subdivision/subdivisionsettings.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

class NormalRefiner {
public:
    explicit NormalRefiner(const NormalRefinementSettings &normRefSettings, const SubdivisionSettings &subdivSettings);

    void refine(Curve &curve, CurvatureType curvatureType);
    void refineSelected(Curve &curve, CurvatureType curvatureType, int idx);

private:
    const NormalRefinementSettings &normRefSettings_;
    ConicSubdivider subdivider_;
    Curve testCurve_;

    real_t calcCurvature(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c, CurvatureType curvatureType) const;

    real_t curvatureAtIdx(Curve &curve, int idx, CurvatureType curvatureType) const;

    real_t smoothnessPenalty(Curve &curve, int idx, CurvatureType curvatureType) const;

    void binarySearchBestNormal(Curve &curve, int idx, CurvatureType curvatureType);
};

} // namespace conis::core
