#include "curve.hpp"

#include <utility>

#include "conis/core/conics/conic.hpp"
#include "conis/core/curve/curveutils.hpp"

namespace conis::core {

Curve::Curve() : Curve({}, {}, false) {}

Curve::Curve(bool closed) : Curve({}, {}, closed) {}

Curve::Curve(std::vector<Vector2DD> coords, bool closed) : Curve(std::move(coords), calcNormals(coords), closed) {}

Curve::Curve(std::vector<Vector2DD> coords, std::vector<Vector2DD> normals, bool closed)
    : closed_(closed),
      coords_(std::move(coords)),
      normals_(std::move(normals)) {
    customNormals_.resize(coords_.size());
    std::fill(customNormals_.begin(), customNormals_.end(), false);
}

std::vector<Vector2DD> Curve::calcNormals(const std::vector<Vector2DD> &coords) const {
    std::vector<Vector2DD> normals;
    int n = int(coords.size());
    normals.resize(n);
    for (int i = 0; i < n; i++) {
        normals[i] = calcNormalAtIndex(coords, normals, i);
    }
    return normals;
}

Vector2DD Curve::calcNormalAtIndex(const std::vector<Vector2DD> &coords,
                                   const std::vector<Vector2DD> &normals,
                                   int i) const {
    int n = int(normals.size());
    Vector2DD a = coords[getPrevIdx(i)];
    Vector2DD b = coords[i];
    Vector2DD c = coords[getNextIdx(i)];
    if (circleNormals_) {
        return CurveUtils::calcNormalOscCircles(a, b, c);
    } else {
        return CurveUtils::calcNormal(a, b, c, areaWeightedNormals_);
    }
}

real_t Curve::curvatureAtIdx(int i, CurvatureType curvatureType) const {
    int n = numPoints();
    const auto &p_1 = coords_[getPrevIdx(i)];
    const auto &p0 = coords_[i];
    const auto &p1 = coords_[getNextIdx(i)];
    return std::abs(CurveUtils::calcCurvature(p_1, p0, p1, curvatureType));
}

int Curve::addPoint(const Vector2DD &p) {
    int idx = findInsertIdx(p);
    coords_.insert(coords_.begin() + idx, p);
    normals_.insert(normals_.begin() + idx, Vector2DD());
    customNormals_.insert(customNormals_.begin() + idx, false);
    normals_[idx] = calcNormalAtIndex(coords_, normals_, idx);
    normals_[getNextIdx(idx)] = calcNormalAtIndex(coords_, normals_, getNextIdx(idx));
    normals_[getPrevIdx(idx)] = calcNormalAtIndex(coords_, normals_, getPrevIdx(idx));
    return idx;
}

void Curve::setVertexPosition(int idx, const Vector2DD &p) {
    coords_[idx] = p;
    if (!customNormals_[idx]) {
        recalculateNormal(idx);
    }
    int nextIdx = getNextIdx(idx);
    if (!customNormals_[nextIdx]) {
        recalculateNormal(nextIdx);
    }
    int prevIdx = getPrevIdx(idx);
    if (!customNormals_[prevIdx]) {
        recalculateNormal(prevIdx);
    }
}

void Curve::setNormal(int idx, const Vector2DD &normal) {
    normals_[idx] = normal;
    customNormals_[idx] = true;
}

void Curve::removePoint(int idx) {
    if (idx < 0 || idx > numPoints()) {
        return;
    }
    coords_.erase(coords_.begin() + idx);
    normals_.erase(normals_.begin() + idx);
    customNormals_.erase(customNormals_.begin() + idx);
    int prevIdx = getPrevIdx(idx);
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
    double currentDist, minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < coords_.size(); k++) {
        currentDist = (coords_[k] - p).norm();
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
    double currentDist, minDist = std::numeric_limits<double>::infinity();
    for (int k = 0; k < coords_.size(); k++) {
        Vector2DD normPos = coords_[k] + normalLength * normals_[k];
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
    int n = coords_.size();
    // Don't loop over the last edge if the curve is not closed
    for (int k = 0; k < n - !isClosed(); k++) {
        const Vector2DD &start = coords_[k];
        const Vector2DD &end = coords_[getNextIdx(k)];
        Vector2DD closestPoint = getClosestPointOnLineSegment(start, end, p);
        double currentDist = (closestPoint - p).norm();
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
    Vector2DD lineDir = end - start;
    double lineLengthSquared = lineDir.dot(lineDir);
    if (lineLengthSquared == 0)
        return start;
    // Project the point onto the line segment
    double t = ((point - start).dot(lineDir)) / lineLengthSquared;
    t = std::max(0.0, std::min(1.0, t)); // Clamp t to the segment [0, 1]
    // Return the closest point on the line segment
    return start + t * lineDir;
}

int Curve::getNextIdx(int idx) const {
    int n = int(coords_.size());
    return closed_ ? (idx + 1) % n : std::min(idx + 1, n - 1);
}

int Curve::getPrevIdx(int idx) const {
    int n = int(coords_.size());
    return closed_ ? (idx - 1 + n) % n : std::max(idx - 1, 0);
}

int Curve::findInsertIdx(const Vector2DD &p) const {
    if (coords_.empty()) {
        return 0;
    }
    int ptIndex = -1;
    double currentDist, minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < coords_.size(); k++) {
        currentDist = CurveUtils::distanceToEdge(coords_[k], coords_[getPrevIdx(k)], p);
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    return ptIndex;
}

void Curve::recalculateNormals(bool areaWeightedNormals, bool circleNormals) {
    areaWeightedNormals_ = areaWeightedNormals;
    circleNormals_ = circleNormals;
    normals_ = calcNormals(coords_);
    for (int i = 0; i < normals_.size(); i++) {
        customNormals_[i] = false;
    }
}

void Curve::recalculateNormal(int idx) {
    customNormals_[idx] = false;
    normals_[idx] = calcNormalAtIndex(coords_, normals_, idx);
}

bool Curve::isClosed() const {
    return closed_;
}

void Curve::setClosed(bool closed) {
    closed_ = closed;
    if (coords_.empty()) {
        return;
    }
    if (!customNormals_[0]) {
        recalculateNormal(0);
    }
    if (!customNormals_[customNormals_.size() - 1]) {
        recalculateNormal(int(customNormals_.size()) - 1);
    }
}

void Curve::translate(const Vector2DD &translation) {
    for (auto &c: coords_) {
        c += translation;
    }
}

int Curve::numPoints() const {
    return coords_.size();
}

void Curve::copyDataTo(Curve &other) const {
    auto &otherCoords = other.getCoords();
    auto &otherNormals = other.getNormals();
    auto &otherCustNormals = other.getCustomNormals();
    other.setClosed(isClosed());
    int n = numPoints();
    // Prevent re-allocating the buffer, so copy it into existing buffer
    // Note that resize does not reduce capacity
    otherCoords.resize(n);
    otherNormals.resize(n);
    otherCustNormals.resize(n);
    std::copy(coords_.begin(), coords_.end(), otherCoords.begin());
    std::copy(normals_.begin(), normals_.end(), otherNormals.begin());
    std::copy(customNormals_.begin(), customNormals_.end(), otherCustNormals.begin());
}

Vector2DD Curve::prevEdge(int idx) const {
    return coords_[getPrevIdx(idx)] - coords_[idx];
}
Vector2DD Curve::nextEdge(int idx) const {
    return coords_[getNextIdx(idx)] - coords_[idx];
}

int Curve::edgePointingDir(int idx) const {
    return std::clamp(vertexPointingDir(idx) + vertexPointingDir(getNextIdx(idx)), -1, 1);
}

int Curve::vertexPointingDir(int idx) const {
    auto ab = prevEdge(idx);
    auto cb = nextEdge(idx);
    real_t cross = ab.x() * cb.y() - ab.y() * cb.x();
    if(cross == 0.0) {
        return 0;
    }
    return cross > 0 ? 1 : -1;
}

} // namespace conis::core