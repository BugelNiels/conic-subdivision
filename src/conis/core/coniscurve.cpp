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
    std::vector<Vector2DD> coords;
    std::vector<Vector2DD> normals;
    // We use a temp subdivider to ensure we can keep const correctness
    // Otherwise the getInflPointCurve updates some local fields of tempSubdivider (which we don't care about)
    // The local field updating is done for performance reasons to prevent allocations, but in this case const
    // correctness is more important
    ConicSubdivider tempSubdivider(subdivSettings_);
    tempSubdivider.getInflPointCurve(controlCurve_, coords, normals);
    std::vector<PatchPoint> patch = subdivider_.extractPatch(coords,
                                                             normals,
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
    // inflPointIndices_ = subdivider_. // TODO
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
    Vector2DD &normal = controlCurve_.getNormals()[idx];
    const auto &coords = controlCurve_.getCoords();
    normal = (p - coords[idx]).normalized();
    if (constrain) {
        int n = controlCurve_.numPoints();
        // Constrain the normal in some sensible bounds
        Vector2DD ab = coords[(idx - 1 + n) % n] - coords[idx];
        ab.normalize();
        Vector2DD cb = coords[(idx + 1) % n] - coords[idx];
        cb.normalize();
        // Calculate the dot products
        float dotLeft = normal.dot(ab);
        float dotRight = normal.dot(cb);

        // Cross product is used to correct normal orientation (always point outside)
        real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
        // Some black magic clamping
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