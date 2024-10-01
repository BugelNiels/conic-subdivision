#pragma once

#include "core/curve/curve.hpp"
#include "core/settings/settings.hpp"
#include "core/vector.hpp"
#include "core/conics/conic.hpp"
#include "core/scenelistener.hpp"

namespace conics::core {

/**
 * @brief The Scene class contains the scene description of the program: a control curve and a subdivision curve.
 */
class Scene {
public:
    Scene(Settings &settings);

    [[nodiscard]] inline const Curve &getControlCurve() const { return controlCurve_; }
    [[nodiscard]] inline const Curve &getSubdivCurve() const { return subdivCurve_; }

    [[nodiscard]] inline Curve &getControlCurve() { return controlCurve_; }
    [[nodiscard]] inline Curve &getSubdivCurve() { return subdivCurve_; }

    void resubdivide();

    void recalculateNormals();

    void refineNormals();

    void refineSelectedNormal();

    void setControlCurve(Curve curve);

    void subdivideCurve(int numSteps);

    Conic getConicAtIndex(int idx) const;

    void addListener(SceneListener* listener);
    void removeListener(SceneListener* listener);
    void notifyListeners();

private:
    std::vector<SceneListener*> listeners;
    Settings &settings_;
    Curve controlCurve_;
    Curve subdivCurve_;
    int lastSubdivLevel_ = 0;
};

} // namespace conics::core