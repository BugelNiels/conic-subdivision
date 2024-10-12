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
    std::vector<PatchPoint> patch = subdivider_.extractPatch(controlCurve_,
                                                             idx,
                                                             subdivSettings_.patchSize);
    return Conic(patch, subdivSettings_.epsilon);
}

void ConisCurve::insertInflectionPoints() {
    controlCurve_ = subdivider_.getInflPointCurve(controlCurve_);
    const auto& customNorms = controlCurve_.getCustomNormals();

    inflPointIndices_.clear();
    for (int i = 0; i < customNorms.size(); i++) {
        if (customNorms[i]) {
            inflPointIndices_.insert(i);
        }
    }
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
    // Note that this does not refine the inflection point normals unless they have been explicitly inserted by the user
    normalRefiner_.refine(controlCurve_, curvatureType);
    resubdivide();
    notifyListeners();
}

void ConisCurve::refineNormal(int idx, CurvatureType curvatureType) {
    normalRefiner_.refineSelected(controlCurve_, curvatureType, idx);
    resubdivide();
    notifyListeners();
}

void ConisCurve::refineNormalsProgressively(CurvatureType curvatureType) {
    // Note that this does not refine the inflection point normals unless they have been explicitly inserted by the user
    const auto refineLambda = [this, curvatureType]() {
        normalRefiner_.refine(controlCurve_, curvatureType);
    };
    const auto waitLambda = [this]() {
        resubdivide();
        notifyListeners();
    };
    AsyncRunner::runAndWait(refineLambda, waitLambda);
}

void ConisCurve::refineNormalProgressively(int idx, CurvatureType curvatureType) {
    const auto refineLambda = [this, idx, curvatureType]() {
        normalRefiner_.refineSelected(controlCurve_, curvatureType, idx);
    };
    const auto waitLambda = [this]() {
        resubdivide();
        notifyListeners();
    };
    AsyncRunner::runAndWait(refineLambda, waitLambda);
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

void ConisCurve::redirectNormalToPoint(int idx, const Vector2DD &p, bool constrain) {
    Vector2DD &normal = controlCurve_.getNormal(idx);
    normal = (p - controlCurve_.getVertex(idx)).normalized();
    // Always allow free movement of the inflection point indices
    if (constrain && inflPointIndices_.count(idx) == 0) {
        int n = controlCurve_.numPoints();
        // Constrain the normal in some sensible bounds
        Vector2DD ab = controlCurve_.prevEdge(idx).normalized();
        Vector2DD cb = controlCurve_.nextEdge(idx).normalized();
        // Calculate the dot products
        real_t dotLeft = normal.dot(ab);
        real_t dotRight = normal.dot(cb);
        // Cross product is used to correct normal orientation (always point outside)
        real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
        // Some black magic clamping. Difficult to explain without a drawing
        if (dotLeft > 0 && dotRight > 0) {
            if (dotLeft > dotRight) {
                normal = {ab.y(), -ab.x()};
            } else {
                normal = {-cb.y(), cb.x()};
            }
            normal = cross < 0 ? -1 * normal : normal;
        } else if (dotLeft > 0) {
            normal = {ab.y(), -ab.x()};
            normal = cross < 0 ? -1 * normal : normal;
        } else if (dotRight > 0) {
            normal = {-cb.y(), cb.x()};
            normal = cross < 0 ? -1 * normal : normal;
        }
    }
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