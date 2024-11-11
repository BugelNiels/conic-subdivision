#pragma once

#include <unordered_set>

#include "conis/core/conics/conic.hpp"
#include "conis/core/curve/curve.hpp"
#include "conis/core/curve/refinement/normalrefinementsettings.hpp"
#include "conis/core/curve/refinement/normalrefiner.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/curve/subdivision/subdivisionsettings.hpp"
#include "conis/core/listener.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

/**
 * @brief The ConisCurve class contains the basic subdivision curve description: a control curve and a subdivision curve.
 */
class ConisCurve {
public:
    ConisCurve(const SubdivisionSettings &subdivSettings, const NormalRefinementSettings &normRefSettings);

    [[nodiscard]] const Curve &getControlCurve() const { return controlCurve_; }
    [[nodiscard]] const Curve &getSubdivCurve() const { return subdivCurve_; }
    [[nodiscard]] int getSubdivLevel() const { return lastSubdivLevel_; }

    void setControlCurve(Curve curve);
    void subdivideCurve(int level);
    int addPoint(const Vector2DD &p);
    void removePoint(int idx);

    void recalculateNormals();
    void recalculateNormal(int idx);

    void insertInflectionPoints();
    void resubdivide();
    void refineNormals(CurvatureType curvatureType);
    void refineNormal(int idx, CurvatureType curvatureType);
    void refineNormalsProgressively(CurvatureType curvatureType);
    void refineNormalProgressively(int idx, CurvatureType curvatureType);

    void setControlCurveClosed(bool closed);
    void setVertexPosition(int idx, const Vector2DD &p);
    void redirectNormalToPoint(int idx, const Vector2DD &p, bool constrain);
    void translate(const Vector2DD &d);

    Conic getConicAtIndex(int idx) const;

    void addListener(Listener *listener);
    void removeListener(Listener *listener);
    void notifyListeners();

private:
    const SubdivisionSettings &subdivSettings_;
    const NormalRefinementSettings &normRefSettings_;
    std::unordered_set<int> inflPointIndices_;
    std::vector<Listener *> listeners;
    ConicSubdivider subdivider_;
    NormalRefiner normalRefiner_;
    Curve controlCurve_;
    Curve subdivCurve_;
    int lastSubdivLevel_ = 0;
};

} // namespace conis::core