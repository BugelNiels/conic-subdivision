#pragma once

#include "conis/core/curve/curve.hpp"
#include "conis/core/curve/curvaturetype.hpp"
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

    real_t smoothnessPenalty(const Curve &curve, int idx, CurvatureType curvatureType) const;
    void binarySearchBestNormal(Curve &curve, int idx, bool inflectionPoint, CurvatureType curvatureType);
};

} // namespace conis::core
