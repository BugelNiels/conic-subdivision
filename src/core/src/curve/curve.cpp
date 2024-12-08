#include <conis/core/curve/curve.hpp>

#include <utility>

#include "conis/core/conics/conic.hpp"
#include "curveutils.hpp"

#include <iostream>

namespace conis::core {

Curve::Curve() : Curve({}, {}, false) {}

Curve::Curve(const bool closed) : Curve({}, {}, closed) {}

Curve::Curve(std::vector<Vector2DD> verts, const bool closed)
    : Curve(std::move(verts), {}, closed) {}

Curve::Curve(std::vector<Vector2DD> verts,
             std::vector<Vector2DD> normals,
             const bool closed)
    : closed_(closed),
      vertices_(std::move(verts)),
      normals_(std::move(normals)) {
    customNormals_.resize(vertices_.size());
    std::fill(customNormals_.begin(), customNormals_.end(), false);
    if (normals_.size() != vertices_.size()) {
        normals_ = calcNormals(vertices_);
    }
}

void Curve::setVertex(const int idx, Vector2DD coord) {
    if (idx < 0 || idx >= static_cast<int>(vertices_.size())) {
        throw std::out_of_range("Index out of bounds in setVertex");
    }
    vertices_[idx] = std::move(coord);
}

void Curve::setNormal(const int idx, Vector2DD normal) {
    if (idx < 0 || idx >= static_cast<int>(normals_.size())) {
        throw std::out_of_range("Index out of bounds in setNormal");
    }
    normals_[idx] = std::move(normal);
}

std::vector<Vector2DD> Curve::calcNormals(
    const std::vector<Vector2DD> &verts) const {
    std::vector<Vector2DD> normals;
    const int n = static_cast<int>(verts.size());
    normals.resize(n);
    for (int i = 0; i < n; i++) {
        normals[i] = calcNormalAtIndex(verts, i);
    }
    return normals;
}

Vector2DD Curve::calcNormalAtIndex(const std::vector<Vector2DD> &verts,
                                   const int i) const {
    const Vector2DD &a = verts[getPrevIdx(i)];
    const Vector2DD &b = verts[i];
    const Vector2DD &c = verts[getNextIdx(i)];
    if (circleNormals_) {
        return CurveUtils::calcNormalOscCircles(a, b, c);
    }
    return CurveUtils::calcNormal(a, b, c, areaWeightedNormals_);
}

real_t Curve::curvatureAtIdx(int idx, const CurvatureType curvatureType) const {
    const auto &p_1 = vertices_[getPrevIdx(idx)];
    const auto &p0 = vertices_[idx];
    const auto &p1 = vertices_[getNextIdx(idx)];
    return std::abs(CurveUtils::calcCurvature(p_1, p0, p1, curvatureType));
}

int Curve::addPoint(const Vector2DD &p) {
    const int idx = findInsertIdx(p);
    vertices_.insert(vertices_.begin() + idx, p);
    normals_.insert(normals_.begin() + idx, Vector2DD());
    customNormals_.insert(customNormals_.begin() + idx, false);
    normals_[idx] = calcNormalAtIndex(vertices_, idx);
    normals_[getNextIdx(idx)] = calcNormalAtIndex(vertices_, getNextIdx(idx));
    normals_[getPrevIdx(idx)] = calcNormalAtIndex(vertices_, getPrevIdx(idx));
    return idx;
}

void Curve::setVertexPosition(const int idx, const Vector2DD &p) {
    vertices_[idx] = p;
    if (!customNormals_[idx]) {
        recalculateNormal(idx);
    }
    const int nextIdx = getNextIdx(idx);
    if (!customNormals_[nextIdx]) {
        recalculateNormal(nextIdx);
    }
    const int prevIdx = getPrevIdx(idx);
    if (!customNormals_[prevIdx]) {
        recalculateNormal(prevIdx);
    }
}

void Curve::setCustomNormal(const int idx, const Vector2DD &normal) {
    normals_[idx] = normal;
    customNormals_[idx] = true;
}

void Curve::removePoint(int idx) {
    if (idx < 0 || idx > numPoints()) {
        return;
    }
    vertices_.erase(vertices_.begin() + idx);
    normals_.erase(normals_.begin() + idx);
    customNormals_.erase(customNormals_.begin() + idx);
    const int prevIdx = getPrevIdx(idx);
    if (!customNormals_[prevIdx]) {
        recalculateNormal(prevIdx);
    }
    if (idx == customNormals_.size()) {
        idx = 0;
    }
    if (!customNormals_[idx]) {
        recalculateNormal(idx);
    }
}

int Curve::findClosestVertex(const Vector2DD &p, const double maxDist) const {
    int ptIndex = -1;
    double currentDist;
    double minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < vertices_.size(); k++) {
        currentDist = (vertices_[k] - p).norm();
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    if (minDist >= maxDist) {
        return -1;
    }

    return ptIndex;
}

// Returns index of the point normal handle
int Curve::findClosestNormal(const Vector2DD &p, const double maxDist, const double normalLength) const {
    int ptIndex = -1;
    double currentDist;
    double minDist = std::numeric_limits<double>::infinity();
    for (int k = 0; k < vertices_.size(); k++) {
        Vector2DD normPos = vertices_[k] + normalLength * normals_[k];
        currentDist = (normPos - p).norm();
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    if (minDist >= maxDist) {
        return -1;
    }

    return ptIndex;
}

int Curve::findClosestEdge(const Vector2DD &p, const double maxDist) const {
    int closestEdgeIndex = -1;
    double minDist = std::numeric_limits<double>::infinity();
    const int n = vertices_.size();
    // Don't loop over the last edge if the curve is not closed
    for (int k = 0; k < n - !isClosed(); k++) {
        const Vector2DD &start = vertices_[k];
        const Vector2DD &end = vertices_[getNextIdx(k)];
        Vector2DD closestPoint = getClosestPointOnLineSegment(start, end, p);
        const double currentDist = (closestPoint - p).norm();
        if (currentDist < minDist) {
            minDist = currentDist;
            closestEdgeIndex = k;
        }
    }
    if (minDist >= maxDist) {
        return -1;
    }

    return closestEdgeIndex;
}

Vector2DD Curve::getClosestPointOnLineSegment(const Vector2DD &start,
                                              const Vector2DD &end,
                                              const Vector2DD &point) const {
    const Vector2DD lineDir = end - start;
    const double lineLengthSquared = lineDir.dot(lineDir);
    if (lineLengthSquared == 0)
        return start;
    // Project the point onto the line segment
    double t = (point - start).dot(lineDir) / lineLengthSquared;
    t = std::max(0.0, std::min(1.0, t)); // Clamp t to the segment [0, 1]
    // Return the closest point on the line segment
    return start + t * lineDir;
}

int Curve::findInsertIdx(const Vector2DD &p) const {
    if (vertices_.empty()) {
        return 0;
    }
    int ptIndex = -1;
    real_t minDist = std::numeric_limits<real_t>::infinity();

    for (int k = 0; k < vertices_.size(); k++) {
        real_t currentDist = CurveUtils::distanceToEdge(vertices_[k], vertices_[getPrevIdx(k)], p);
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    return ptIndex;
}

int Curve::getNextIdx(const int idx) const {
    const int n = static_cast<int>(vertices_.size());
    if (closed_) {
        return idx + 1 < n ? idx + 1 : 0;
    }
    return std::min(idx + 1, n - 1);
}

int Curve::getPrevIdx(const int idx) const {
    const int n = static_cast<int>(vertices_.size());
    if (closed_) {
        return idx - 1 >= 0 ? idx - 1 : n - 1;
    }
    return std::max(idx - 1, 0);
}

void Curve::recalculateNormals(const bool areaWeightedNormals, const bool circleNormals) {
    areaWeightedNormals_ = areaWeightedNormals;
    circleNormals_ = circleNormals;
    normals_ = calcNormals(vertices_);
    for (int i = 0; i < normals_.size(); i++) {
        customNormals_[i] = false;
    }
}

void Curve::recalculateNormal(const int idx) {
    customNormals_[idx] = false;
    normals_[idx] = calcNormalAtIndex(vertices_, idx);
}

bool Curve::isClosed() const {
    return closed_;
}

void Curve::setClosed(const bool closed, bool recalculate) {
    closed_ = closed;
    if (vertices_.empty() || !recalculate) {
        return;
    }
    if (!customNormals_[0]) {
        recalculateNormal(0);
    }
    if (!customNormals_[customNormals_.size() - 1]) {
        recalculateNormal(static_cast<int>(customNormals_.size()) - 1);
    }
}

void Curve::translate(const Vector2DD &translation) {
    for (auto &c: vertices_) {
        c += translation;
    }
}

int Curve::numPoints() const {
    return vertices_.size();
}

void Curve::copyDataTo(Curve &other) const {
    auto &otherCoords = other.getVertices();
    auto &otherNormals = other.getNormals();
    auto &otherCustNormals = other.getCustomNormals();
    other.setClosed(isClosed(), false);
    const int n = numPoints();
    // Prevent re-allocating the buffer, so copy it into existing buffer
    // Note that resize does not reduce capacity
    otherCoords.resize(n);
    otherNormals.resize(n);
    otherCustNormals.resize(n);
    std::copy(vertices_.begin(), vertices_.end(), otherCoords.begin());
    std::copy(normals_.begin(), normals_.end(), otherNormals.begin());
    std::copy(customNormals_.begin(), customNormals_.end(), otherCustNormals.begin());
}

Vector2DD Curve::prevEdge(const int idx) const {
    return vertices_[getPrevIdx(idx)] - vertices_[idx];
}
Vector2DD Curve::nextEdge(const int idx) const {
    return vertices_[getNextIdx(idx)] - vertices_[idx];
}

int Curve::edgePointingDir(const int idx) const {
    if (vertexPointingDir(idx) == 1 || vertexPointingDir(getNextIdx(idx)) == 1) {
        return 1;
    }
    return -1;
}

int Curve::vertexPointingDir(const int idx) const {
    const auto ab = prevEdge(idx);
    const auto cb = nextEdge(idx);
    const real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
    if (cross == 0.0) {
        return 0;
    }
    return cross > 0 ? 1 : -1;
}

} // namespace conis::core