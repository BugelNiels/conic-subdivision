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
}

void Scene::setControlCurve(Curve controlCurve) {
    controlCurve_ = controlCurve;
    resubdivide();
}

void Scene::recalculateNormals() {
    controlCurve_.recalculateNormals(settings_.areaWeightedNormals, settings_.circleNormals);
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

int Scene::addPoint(const Vector2DD &p) {
    int idx = controlCurve_.addPoint(p);
    resubdivide();
    return idx;
}

void Scene::removePoint(int idx) {
    controlCurve_.removePoint(idx);
    resubdivide();
}

void Scene::setControlCurveClosed(bool closed) {
    controlCurve_.setClosed(closed);
    resubdivide();
}

void Scene::recalculateNormal(int idx) {
    controlCurve_.recalculateNormal(idx);
    resubdivide();
}

void Scene::setVertexPosition(int idx, const Vector2DD &p) {
    controlCurve_.setVertexPosition(idx, p);
    resubdivide();
}
void Scene::redirectNormalToPoint(int idx, const Vector2DD &p) {
    controlCurve_.redirectNormalToPoint(idx, p);
    resubdivide();
}
void Scene::translate(const Vector2DD& d) {
    controlCurve_.translate(d);
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