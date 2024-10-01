#include "curve.hpp"

#include <utility>

#include "core/conics/conic.hpp"

Curve::Curve() : Curve({}, {}, false) {}

Curve::Curve(bool closed) : Curve({}, {}, closed) {}

Curve::Curve(std::vector<Vector2DD> coords, bool closed)
    : Curve(std::move(coords), calcNormals(coords), closed) {}

Curve::Curve(std::vector<Vector2DD> coords, std::vector<Vector2DD> normals, bool closed)
    : closed_(closed),
      coords_(std::move(coords)),
      normals_(std::move(normals)) {
    customNormals_.resize(coords_.size());
    std::fill(customNormals_.begin(), customNormals_.end(), false);
}

Vector2DD Curve::calcNormal(const Vector2DD &a,
                            const Vector2DD &b,
                            const Vector2DD &c,
                            bool areaWeighted) const {
    if (a == b) {
        Vector2DD normal = c - b;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    if (b == c) {
        Vector2DD normal = b - a;
        normal.x() *= -1;
        return Vector2DD(normal.y(), normal.x()).normalized();
    }
    Vector2DD t1 = (a - b);
    t1 = {-t1.y(), t1.x()};
    Vector2DD t2 = (b - c);
    t2 = {-t2.y(), t2.x()};
    if (!areaWeighted) {
        t1.normalize();
        t2.normalize();
    }
    return (t1 + t2).normalized();
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
    int nextIdx;
    int prevIdx;
    if (closed_) {
        prevIdx = (i - 1 + n) % n;
        nextIdx = (i + 1) % n;
    } else {
        prevIdx = std::max(0, i - 1);
        nextIdx = std::min(i + 1, n - 1);
    }
    Vector2DD a = coords[prevIdx];
    Vector2DD b = coords[i];
    Vector2DD c = coords[nextIdx];
    if (circleNormals_) {
        if (a == b) {
            Vector2DD normal = c - b;
            normal.x() *= -1;
            return Vector2DD(normal.y(), normal.x()).normalized();
        } else if (b == c) {
            Vector2DD normal = b - a;
            normal.x() *= -1;
            return Vector2DD(normal.y(), normal.x()).normalized();
        } else {
            real_t d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) +
                            c.x() * (a.y() - b.y()));
            real_t ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                         (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                         (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                        d;
            real_t uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                         (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                         (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                        d;
            Vector2DD oscCircleCenter = Vector2DD(ux, uy);
            Vector2DD norm = (oscCircleCenter - b).normalized();

            Vector2DD check = calcNormal(a, b, c, areaWeightedNormals_);
            if (check.dot(normals[i]) < 0) {
                norm *= -1;
            }
            return norm;
        }
    } else {
        return calcNormal(a, b, c, areaWeightedNormals_);
    }
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

// Sets the normal to of the vertex at the provided index to point towards the provided point
void Curve::redirectNormalToPoint(int idx, const Vector2DD &p) {
    normals_[idx] = p - coords_[idx];
    normals_[idx].normalize();
    customNormals_[idx] = true;
}

void Curve::removePoint(int idx) {
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

int Curve::getNextIdx(int idx) const {
    int n = int(coords_.size());
    return closed_ ? (idx + 1) % n : std::min(idx + 1, n - 1);
}

int Curve::getPrevIdx(int idx) const {
    int n = int(coords_.size());
    return closed_ ? (idx - 1 + n) % n : std::max(idx - 1, 0);
}

static double distanceToEdge(const Vector2DD &A, const Vector2DD &B, const Vector2DD &P) {
    Vector2DD AB = B - A;
    Vector2DD AP = P - A;
    double AB_len2 = AB.squaredNorm();
    double t = AP.dot(AB) / AB_len2;
    t = std::max(0.0, std::min(1.0, t)); // clamp to [0, 1]
    Vector2DD Q = A + t * AB;
    return std::hypot(P.x() - Q.x(), P.y() - Q.y());
}

int Curve::findInsertIdx(const Vector2DD &p) const {
    if (coords_.empty()) {
        return 0;
    }
    int ptIndex = -1;
    double currentDist, minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < coords_.size(); k++) {
        currentDist = distanceToEdge(coords_[k], coords_[getPrevIdx(k)], p);
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    return ptIndex;
}

// Returns index of the point normal handle
// TODO: move this outside of this class?
int Curve::findClosestNormal(const Vector2DD &p, const double maxDist, const double normalLength) const {
    int ptIndex = -1;
    double currentDist, minDist = 4;

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
