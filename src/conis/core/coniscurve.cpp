#include "coniscurve.hpp"

#include <iostream>

#include "conis/core/conics/conic.hpp"
#include "conis/core/curve/subdivision/conicsubdivider.hpp"
#include "conis/core/util/asyncrunner.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

ConisCurve::ConisCurve(const SubdivisionSettings &subdivSettings, const NormalRefinementSettings &normRefSettings)
    : subdivSettings_(subdivSettings),
      normRefSettings_(normRefSettings),
      subdivider_(subdivSettings_),
      normalRefiner_(normRefSettings, subdivSettings) {}

void ConisCurve::subdivideCurve(int numSteps) {
    lastSubdivLevel_ = numSteps;
    controlCurve_.copyDataTo(subdivCurve_);
    subdivider_.subdivide(subdivCurve_, numSteps);
    notifyListeners();
}

Conic ConisCurve::getConicAtIndex(int idx) const {
    std::vector<PatchPoint> patch = subdivider_.extractPatch(controlCurve_.getCoords(),
                                                             controlCurve_.getNormals(),
                                                             idx,
                                                             subdivSettings_.patchSize,
                                                             controlCurve_.isClosed());
    return Conic(patch, subdivSettings_.epsilon);
}

void ConisCurve::insertInflectionPoints() {
    std::vector<Vector2DD> coords;
    std::vector<Vector2DD> normals;
    controlCurve_.setCustomNormals(subdivider_.getInflPointCurve(controlCurve_, coords, normals));
    controlCurve_.setCoords(coords);
    controlCurve_.setNormals(normals);
    resubdivide();
}

void ConisCurve::resubdivide() {
    subdivideCurve(lastSubdivLevel_);
}

void ConisCurve::setControlCurve(Curve controlCurve) {
    controlCurve_ = controlCurve;
    lastSubdivLevel_ = 0;
    resubdivide();
}

void ConisCurve::recalculateNormals() {
    controlCurve_.recalculateNormals(subdivSettings_.areaWeightedNormals, subdivSettings_.circleNormals);
    resubdivide();
}

void ConisCurve::refineNormals(CurvatureType curvatureType) {
    const auto refineLambda = [this, curvatureType]() {
        normalRefiner_.refine(controlCurve_, curvatureType);
    };
    const auto waitLambda = [this]() {
        resubdivide();
        notifyListeners();
    };

    AsyncRunner::runAndWait(refineLambda, waitLambda);
}

void ConisCurve::refineNormal(int idx, CurvatureType curvatureType) {
    const auto refineLambda = [this, idx, curvatureType]() {
        normalRefiner_.refineSelected(controlCurve_, curvatureType, idx);
    };
    const auto waitLambda = [this]() {
        resubdivide();
        notifyListeners();
    };

    AsyncRunner::runAndWait(refineLambda, waitLambda);
    resubdivide();
}

int ConisCurve::addPoint(const Vector2DD &p) {
    int idx = controlCurve_.addPoint(p);
    resubdivide();
    return idx;
}

void ConisCurve::removePoint(int idx) {
    controlCurve_.removePoint(idx);
    resubdivide();
}

void ConisCurve::setControlCurveClosed(bool closed) {
    controlCurve_.setClosed(closed);
    resubdivide();
}

void ConisCurve::recalculateNormal(int idx) {
    controlCurve_.recalculateNormal(idx);
    resubdivide();
}

void ConisCurve::setVertexPosition(int idx, const Vector2DD &p) {
    controlCurve_.setVertexPosition(idx, p);
    resubdivide();
}
void ConisCurve::redirectNormalToPoint(int idx, const Vector2DD &p) {
    controlCurve_.redirectNormalToPoint(idx, p);
    resubdivide();
}
void ConisCurve::translate(const Vector2DD &d) {
    controlCurve_.translate(d);
    resubdivide();
}

void ConisCurve::addListener(Listener *listener) {
    if (listener == nullptr) {
        return;
    }
    if (std::find(listeners.begin(), listeners.end(), listener) == listeners.end()) {
        listeners.emplace_back(listener);
    }
}
void ConisCurve::removeListener(Listener *listener) {
    if (listener == nullptr) {
        return;
    }
    auto it = std::remove(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end()) {
        listeners.erase(it, listeners.end());
    }
}
void ConisCurve::notifyListeners() {
    for (auto *listener: listeners) {
        if (listener) {
            listener->onListenerUpdated();
        }
    }
}

} // namespace conis::core