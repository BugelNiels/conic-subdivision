#include "scene.hpp"

#include "conis/core/conics/conic.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

Scene::Scene(const SubdivisionSettings &subdivSettings, const NormalRefinementSettings &normRefSettings)
    : subdivSettings_(subdivSettings),
      normRefSettings_(normRefSettings),
      subdivider_(subdivSettings_),
      normalRefiner_(normRefSettings, subdivSettings) {}

void Scene::subdivideCurve(int numSteps) {
    lastSubdivLevel_ = numSteps;
    const auto &coords = controlCurve_.getCoords();
    const auto &normals = controlCurve_.getNormals();
    auto &subdivCoords = subdivCurve_.getCoords();
    auto &subdivNormals = subdivCurve_.getNormals();
    subdivCurve_.setClosed(controlCurve_.isClosed());
    int n = coords.size();
    // Prevent re-allocating the buffer, so copy it into existing buffer
    subdivCoords.resize(n); // Note that resize does not reduce capacity
    subdivNormals.resize(n);
    subdivCurve_.getCustomNormals().resize(n);
    std::copy(coords.begin(), coords.end(), subdivCoords.begin());
    std::copy(normals.begin(), normals.end(), subdivNormals.begin());

    subdivider_.subdivide(subdivCurve_, numSteps);
    notifyListeners();
}

Conic Scene::getConicAtIndex(int idx) const {
    std::vector<PatchPoint> patch = subdivider_.extractPatch(controlCurve_.getCoords(),
                                                             controlCurve_.getNormals(),
                                                             idx,
                                                             subdivSettings_.patchSize,
                                                             controlCurve_.isClosed());
    return Conic(patch, subdivSettings_.epsilon);
}

void Scene::resubdivide() {
    subdivideCurve(lastSubdivLevel_);
}

void Scene::setControlCurve(Curve controlCurve) {
    controlCurve_ = controlCurve;
    lastSubdivLevel_ = 0;
    resubdivide();
}

void Scene::recalculateNormals() {
    controlCurve_.recalculateNormals(subdivSettings_.areaWeightedNormals, subdivSettings_.circleNormals);
    resubdivide();
}

void Scene::refineNormals() {
    normalRefiner_.refine(controlCurve_);
    resubdivide();
}

void Scene::refineNormal(int idx) {
    normalRefiner_.refineSelected(controlCurve_, idx);
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
void Scene::translate(const Vector2DD &d) {
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

} // namespace conis::core