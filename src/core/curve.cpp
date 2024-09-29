#include "curve.hpp"

#include <utility>

#include "core/conics/conic.hpp"
#include "settings/settings.hpp"

Curve::Curve(const Settings &settings)
    : settings_(settings),
      closed_(true) {}

Curve::Curve(const Settings &settings,
                                   std::vector<Vector2DD> coords,
                                   bool closed)
    : settings_(settings),
      closed_(closed),
      coords(std::move(coords)) {
    normals_ = calcNormals(coords_);
    customNormals_.resize(normals_.size());
    std::fill(customNormals_.begin(), customNormals_.end(), false);
}

Curve::Curve(const Settings &settings,
                                   std::vector<Vector2DD> coords,
                                   std::vector<Vector2DD> normals,
                                   bool closed)
    : settings_(settings),
      closed_(closed),
      netCoords_(std::move(coords)),
      netNormals_(std::move(normals)),
      subdivider(settings_) {
    int size = netCoords_.size();
    customNormals_.resize(size);
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
    if (settings_.circleNormals) {
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

            Vector2DD check = calcNormal(a, b, c, settings_.areaWeightedNormals);
            if (check.dot(normals[i]) < 0) {
                norm *= -1;
            }
            return norm;
        }
    } else {
        return calcNormal(a, b, c, settings_.areaWeightedNormals);
    }
}

int Curve::addPoint(const Vector2DD &p) {
    int idx = findInsertIdx(p);
    netCoords_.insert(netCoords_.begin() + idx, p);
    netNormals_.insert(netNormals_.begin() + idx, Vector2DD());
    customNormals_.insert(customNormals_.begin() + idx, false);
    netNormals_[idx] = calcNormalAtIndex(netCoords_, netNormals_, idx);
    netNormals_[getNextIdx(idx)] = calcNormalAtIndex(netCoords_, netNormals_, getNextIdx(idx));
    netNormals_[getPrevIdx(idx)] = calcNormalAtIndex(netCoords_, netNormals_, getPrevIdx(idx));
    reSubdivide();
    return idx;
}

void Curve::setVertexPosition(int idx, const Vector2DD &p) {
    netCoords_[idx] = p;
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
    reSubdivide();
}

void Curve::setNormalPosition(int idx, const Vector2DD &p) {
    netNormals_[idx] = p - netCoords_[idx];
    netNormals_[idx].normalize();
    customNormals_[idx] = true;
    reSubdivide();
}

void Curve::removePoint(int idx) {
    netCoords_.erase(netCoords_.begin() + idx);
    netNormals_.erase(netNormals_.begin() + idx);
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
    reSubdivide();
}

int Curve::findClosestVertex(const Vector2DD &p, const double maxDist) const {
    int ptIndex = -1;
    double currentDist, minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < netCoords_.size(); k++) {
        currentDist = (netCoords_[k] - p).norm();
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
    int n = int(netCoords_.size());
    return closed_ ? (idx + 1) % n : std::min(idx + 1, n - 1);
}

int Curve::getPrevIdx(int idx) const {
    int n = int(netCoords_.size());
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
    if (netCoords_.empty()) {
        return 0;
    }
    int ptIndex = -1;
    double currentDist, minDist = std::numeric_limits<double>::infinity();

    for (int k = 0; k < netCoords_.size(); k++) {
        currentDist = distanceToEdge(netCoords_[k], netCoords_[getPrevIdx(k)], p);
        if (currentDist < minDist) {
            minDist = currentDist;
            ptIndex = k;
        }
    }
    return ptIndex;
}

// Returns index of the point normal handle
int Curve::findClosestNormal(const Vector2DD &p, const double maxDist) const {
    int ptIndex = -1;
    double currentDist, minDist = 4;

    for (int k = 0; k < netCoords_.size(); k++) {
        Vector2DD normPos = netCoords_[k] + settings_.normalLength * netNormals_[k];
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

void Curve::reSubdivide() {
    subdivide(subdivisionLevel_);
}

void Curve::subdivide(int level) {
    subdivider.subdivide(this, level);
    subdivisionLevel_ = level;
}

void Curve::recalculateNormals() {
    netNormals_ = calcNormals(netCoords_);
    for (int i = 0; i < netNormals_.size(); i++) {
        customNormals_[i] = false;
    }
}

void Curve::recalculateNormal(int idx) {
    customNormals_[idx] = false;
    netNormals_[idx] = calcNormalAtIndex(netCoords_, netNormals_, idx);
}

bool Curve::isClosed() const {
    return closed_;
}

void Curve::setClosed(bool closed) {
    closed_ = closed;
    if (netCoords_.empty()) {
        return;
    }
    if (!customNormals_[0]) {
        recalculateNormal(0);
    }
    if (!customNormals_[customNormals_.size() - 1]) {
        recalculateNormal(int(customNormals_.size()) - 1);
    }
    reSubdivide();
}

void Curve::applySubdivision() {
    netCoords_ = curveCoords_;
    netNormals_ = curveNormals_;
    customNormals_.resize(netCoords_.size());
    subdivisionLevel_ = 0;
}

void Curve::insertInflPoints() {
    std::vector<Vector2DD> coords;
    std::vector<Vector2DD> norms;
    std::vector<bool> customNorms;
    subdivider.insertInflPoints(this, coords, norms, customNorms);

    netCoords_ = coords;
    netNormals_ = norms;
    customNormals_ = customNorms;
}

void Curve::translate(const Vector2DD &translation) {
    for (auto &c: netCoords_) {
        c += translation;
    }
    reSubdivide();
}

Conic Curve::getConicAtIndex(int idx) {
    std::vector<PatchPoint> patch = subdivider.extractPatch(netCoords_,
                                                            netNormals_,
                                                            idx,
                                                            settings_.patchSize);
    return Conic(patch, settings_);
}

int Curve::numPoints() const {
    return subdivisionLevel_ == 0 ? netCoords_.size() : curveCoords_.size();
}
