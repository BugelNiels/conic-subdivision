#include "scene.hpp"

#include "core/conics/conic.hpp"
#include "core/curve/subdivision/conicsubdivider.hpp"
#include "core/settings/settings.hpp"
#include "core/vector.hpp"

namespace conics::core {

Scene::Scene(Settings &settings) : settings_(settings) {}

void Scene::subdivideCurve(int numSteps) {
    // TODO: make more efficient
    lastSubdivLevel_ = numSteps;
    ConicSubdivider subdivider(settings_);
    subdivCurve_ = controlCurve_;
    subdivider.subdivide(subdivCurve_, numSteps);
    notifyListeners();
}

Conic Scene::getConicAtIndex(int idx) const {
    ConicSubdivider subdivider(settings_);
    std::vector<PatchPoint> patch = subdivider.extractPatch(controlCurve_,
                                                            idx,
                                                            settings_.patchSize);
    return Conic(patch, settings_);
}

void Scene::resubdivide() {
    subdivideCurve(lastSubdivLevel_);
    notifyListeners();
}

void Scene::setControlCurve(Curve controlCurve) {
    controlCurve_ = controlCurve;
    notifyListeners();
}

void Scene::recalculateNormals() {
    controlCurve_.recalculateNormals();
    resubdivide();
}

void Scene::refineNormals() {
    // controlCurve_.refineNormals(settings_.maxRefinementIterations);
    resubdivide();
}

void Scene::refineSelectedNormal() {
    // TODO
    // subCurve_->refineSelectedNormal(settings_.maxRefinementIterations);
    resubdivide();
}

void Scene::addListener(SceneListener *listener) {
    if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end()) {
        listeners.emplace_back(listener);
    }
}
void Scene::removeListener(SceneListener *listener) {
    auto it = std::remove(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end()) {
        listeners.erase(it, listeners.end());
    }
}
void Scene::notifyListeners() {
    for (auto *listener: listeners) {
        if (listener) {
            listener->sceneUpdated();
        }
    }
}

} // namespace conics::core