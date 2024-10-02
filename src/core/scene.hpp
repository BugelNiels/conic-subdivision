#pragma once

#include "core/conics/conic.hpp"
#include "core/curve/curve.hpp"
#include "core/curve/subdivision/subdivider.hpp"
#include "core/scenelistener.hpp"
#include "core/settings/settings.hpp"
#include "core/vector.hpp"

namespace conics::core {

/**
 * @brief The Scene class contains the scene description of the program: a control curve and a subdivision curve.
 */
class Scene {
public:
    Scene(Settings &settings);

    [[nodiscard]] inline const Curve &getControlCurve() const { return controlCurve_; }
    [[nodiscard]] inline const Curve &getSubdivCurve() const { return subdivCurve_; }

    Conic getConicAtIndex(int idx) const;

    void resubdivide();
    void recalculateNormals();
    void recalculateNormal(int idx);
    void refineNormals();
    void refineSelectedNormal();

    void setControlCurve(Curve curve);
    void subdivideCurve(int numSteps);

    int addPoint(const Vector2DD &p);
    void removePoint(int idx);

    void setControlCurveClosed(bool closed);
    void setVertexPosition(int idx, const Vector2DD &p);
    void redirectNormalToPoint(int idx, const Vector2DD &p);
    void translate(const Vector2DD &d);

    void addListener(SceneListener *listener);
    void removeListener(SceneListener *listener);
    void notifyListeners();

private:
    std::vector<SceneListener *> listeners;
    std::unique_ptr<Subdivider> subdivider_;
    Settings &settings_;
    Curve controlCurve_;
    Curve subdivCurve_;
    int lastSubdivLevel_ = 0;
};

} // namespace conics::core