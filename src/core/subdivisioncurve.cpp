#include "subdivisioncurve.hpp"

#include <utility>

#include "settings.hpp"
#include "core/conics/conic.hpp"


SubdivisionCurve::SubdivisionCurve(const Settings &settings)
        : settings_(settings),
          closed_(true) {

}


SubdivisionCurve::SubdivisionCurve(const Settings &settings, QVector<Vector2DD> coords, bool closed)
        : settings_(settings),
          closed_(closed),
          netCoords_(std::move(coords)) {
    netNormals_ = calcNormals(netCoords_);
    customNormals_.resize(netNormals_.size());
    customNormals_.fill(false);
}


SubdivisionCurve::SubdivisionCurve(const Settings &settings, QVector<Vector2DD> coords, QVector<Vector2DD> normals,
                                   bool closed)
        : settings_(settings),
          closed_(closed),
          netCoords_(std::move(coords)),
          netNormals_(std::move(normals)) {
    customNormals_.resize(netNormals_.size());
    customNormals_.fill(false);
}

Vector2DD calcNormal(const Vector2DD &a, const Vector2DD &b,
                     const Vector2DD &c, bool areaWeighted) {
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

QVector<Vector2DD> SubdivisionCurve::calcNormals(
        const QVector<Vector2DD> &coords) const {
    QVector<Vector2DD> normals;
    int n = int(coords.size());
    normals.resize(n);
    for (int i = 0; i < n; i++) {
        calcNormalAtIndex(coords, normals, i);
    }
    return normals;
}

void SubdivisionCurve::calcNormalAtIndex(const QVector<Vector2DD> &coords, QVector<Vector2DD> &normals, int i) const {
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
            normals[i] = Vector2DD(normal.y(), normal.x()).normalized();
        } else if (b == c) {
            Vector2DD normal = b - a;
            normal.x() *= -1;
            normals[i] = Vector2DD(normal.y(), normal.x()).normalized();
        } else {
            double d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) +
                            c.x() * (a.y() - b.y()));
            double ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                         (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                         (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                        d;
            double uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                         (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                         (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                        d;
            Vector2DD oscCircleCenter = Vector2DD(ux, uy);
            normals[i] = (oscCircleCenter - b).normalized();

            Vector2DD check = calcNormal(a, b, c, settings_.areaWeightedNormals);
            if (check.dot(normals[i]) < 0) {
                normals[i] *= -1;
            }
        }
    } else {
        normals[i] = calcNormal(a, b, c, settings_.areaWeightedNormals);
    }
}

/**
 * @brief SubdivisionCurve::addPoint Adds a point to the control net.
 * @param p The point to add to the control net.
 */
int SubdivisionCurve::addPoint(const Vector2DD &p) {
    int idx = findInsertIdx(p);
    netCoords_.insert(idx, p);
    netNormals_.insert(idx, Vector2DD());
    customNormals_.insert(idx, false);
    calcNormalAtIndex(netCoords_, netNormals_, idx);
    calcNormalAtIndex(netCoords_, netNormals_, getNextIdx(idx));
    calcNormalAtIndex(netCoords_, netNormals_, getPrevIdx(idx));
    reSubdivide();
    return idx;
}

/**
 * @brief SubdivisionCurve::setVertexPosition Changes the position of a point in
 * the control net.
 * @param idx The index of the point to update the position of.
 * @param p The new position of the point.
 */
void SubdivisionCurve::setVertexPosition(int idx, const Vector2DD &p) {
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

void SubdivisionCurve::setNormalPosition(int idx, const Vector2DD &p) {
    netNormals_[idx] = p - netCoords_[idx];
    netNormals_[idx].normalize();
    customNormals_[idx] = true;
    reSubdivide();
}

/**
 * @brief SubdivisionCurve::removePoint Removes a point from the control net.
 * @param idx The index of the point to remove.
 */
void SubdivisionCurve::removePoint(int idx) {
    netCoords_.remove(idx);
    netNormals_.remove(idx);
    customNormals_.remove(idx);
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

/**
 * @brief SubdivisionCurve::findClosestVertex Finds the index of the closest point in
 * the control net to the provided point.
 * @param p The point to find the closest point to.
 * @param maxDist The maximum distance a point and the provided point can have.
 * Is a value between 0 and 1.
 * @return The index of the closest point to the provided point. Returns -1 if
 * no point was found within the maximum distance.
 */
int SubdivisionCurve::findClosestVertex(const Vector2DD &p, const double maxDist) {
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

int SubdivisionCurve::getNextIdx(int idx) {
    int n = int(netCoords_.size());
    return closed_ ? (idx + 1) % n : std::min(idx + 1, n - 1);
}

int SubdivisionCurve::getPrevIdx(int idx) {
    int n = int(netCoords_.size());
    return closed_ ? (idx - 1 + n) % n : std::max(idx - 1, 0);
}

double distanceToEdge(const Vector2DD &A, const Vector2DD &B, const Vector2DD &P) {
    Vector2DD AB = B - A;
    Vector2DD AP = P - A;
    double AB_len2 = AB.squaredNorm();
    double t = AP.dot(AB) / AB_len2;
    t = std::max(0.0, std::min(1.0, t)); // clamp to [0, 1]
    Vector2DD Q = A + t * AB;
    return std::hypot(P.x() - Q.x(), P.y() - Q.y());
}

int SubdivisionCurve::findInsertIdx(const Vector2DD &p) {
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
int SubdivisionCurve::findClosestNormal(const Vector2DD &p, const double maxDist) {
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

void SubdivisionCurve::reSubdivide() {
    subdivide(subdivisionLevel_);
}

/**
 * @brief SubdivisionCurve::subdivide Subdivides the curve a number of times.
 * @param level The number of times the curve should be subdivided
 */
void SubdivisionCurve::subdivide(int level) {
    if (int(netCoords_.size()) == 0) {
        return;
    }
    if (settings_.convexitySplit) {

        QVector<Vector2DD> coords;
        QVector<Vector2DD> norms;
        QVector<bool> customNorms;
        knotCurve(coords, norms, customNorms);
        QVector<double> stabilities;
        stabilities.resize(coords.size());
        subdivide(coords, norms, stabilities, level);
    } else {
        stability.resize(netCoords_.size());
        subdivide(netCoords_, netNormals_, stability, level);
    }

    subdivisionLevel_ = level;
}

bool arePointsCollinear(const Vector2DD &p1, const Vector2DD &p2, const Vector2DD &p3) {
    return p1.x() * (p2.y() - p3.y()) + p2.x() * (p3.y() - p1.y()) + p3.x() * (p1.y() - p2.y()) == 0.0;
}

/**
 * @brief SubdivisionCurve::subdivide Recursive subdivision function. Subdivides
 * the provided points recursively a number of times according to the
 * subdivision mask.
 * @param points The points to be subdivided
 * @param level The number of times the points should be subdivided
 */
void SubdivisionCurve::subdivide(const QVector<Vector2DD> &points,
                                 const QVector<Vector2DD> &normals,
                                 const QVector<double> &stabilities, int level) {
    // base case
    if (level == 0) {
        curveCoords_ = points;
        curveNormals_ = normals;
        stability = stabilities;
        return;
    }
    int n = int(points.size()) * 2 - 1;
    if (closed_) {
        n += 1;
    }
    QVector<Vector2DD> newPoints(n);
    QVector<Vector2DD> newNormals(n);
    QVector<double> newStabilities(n);

    // set vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
        newStabilities[i] = stabilities[i / 2];
    }
    // set new points
    int patchSize = 6;
    QVector<int> indices;
    indices.reserve(patchSize);
    for (int i = 1; i < n; i += 2) {
        QVector<Vector2DD> patchCoords;
        QVector<Vector2DD> patchNormals;

        extractPatch(points, normals, indices, i, patchCoords, patchNormals);

        int prevIdx = (i - 1 + n) % n;
        int nextIdx = (i + 1) % n;

        const Vector2DD origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0;
        Vector2DD dir;
        if (settings_.edgeTangentSample) {
            dir = newPoints[prevIdx] - newPoints[nextIdx];
            dir = {-dir.y(), dir.x()};
        } else {
            dir = newNormals[prevIdx] + newNormals[nextIdx];
        }
        // Note that dir is not normalized!
        Conic conic(patchCoords, patchNormals, settings_);
        Vector2DD sampledPoint;
        Vector2DD sampledNormal;

        const bool valid = conic.sample(origin, dir, sampledPoint, sampledNormal);

        if (!valid) {
            sampledPoint = origin;
            sampledNormal = dir;
        }

        newPoints[i] = sampledPoint;
        newNormals[i] = sampledNormal;
        newStabilities[i] = conic.getStability();
    }
    if (settings_.recalculateNormals) {
        newNormals = calcNormals(newPoints);
    }
    knotIndices.clear();
    subdivide(newPoints, newNormals, newStabilities, level - 1);
}

void SubdivisionCurve::extractPatch(const QVector<Vector2DD> &points, const QVector<Vector2DD> &normals,
                                    QVector<int> &indices, int i, QVector<Vector2DD> &patchCoords,
                                    QVector<Vector2DD> &patchNormals) const {
    int pIdx = i / 2;

    int size = int(points.size());

    indices.clear();
    // Two edge points
    indices.append(pIdx);
    // p_2-p_1-p0-p1-p2-p3
    if (closed_) {
        bool startKnotPoint = knotIndices.contains(pIdx);
        bool endKnotPoint = knotIndices.contains((pIdx + 1) % size);
        indices.append((pIdx + 1) % size);
        if (!startKnotPoint) {
            indices.append((pIdx - 1 + size) % size);
        } else {
            indices.append((pIdx + 2) % size);
        }
        if (!endKnotPoint) {
            indices.append((pIdx + 2) % size);
        } else {
            indices.append((pIdx - 1 + size) % size);
        }
    } else {
        indices.append(pIdx + 1);
        if (pIdx + 2 < size) {
            indices.append(pIdx + 2);
        }
        if (pIdx - 1 >= 0) {
            indices.append(pIdx - 1);
        }
    }
    for (int index: indices) {
        patchCoords.append(points[index]);
        patchNormals.append(normals[index]);
    }
}

void SubdivisionCurve::recalculateNormals() {
    netNormals_ = calcNormals(netCoords_);
    for (int i = 0; i < netNormals_.size(); i++) {
        customNormals_[i] = false;
    }
}

void SubdivisionCurve::recalculateNormal(int idx) {
    customNormals_[idx] = false;
    calcNormalAtIndex(netCoords_, netNormals_, idx);
}


bool SubdivisionCurve::isClosed() const {
    return closed_;
}

void SubdivisionCurve::setClosed(bool closed) {
    closed_ = closed;
    if (!customNormals_[0]) {
        recalculateNormal(0);
    }
    if (!customNormals_[customNormals_.size() - 1]) {
        recalculateNormal(int(customNormals_.size()) - 1);
    }
    reSubdivide();
}

bool SubdivisionCurve::areInSameHalfPlane(const Vector2DD &v0, const Vector2DD &v1, const Vector2DD &v2,
                                          const Vector2DD &v3) const {
    Vector2DD v1v3 = v3 - v1;
    Vector2DD v1v0 = v0 - v1;
    if (v1v0.squaredNorm() == 0.0 || v1v3.squaredNorm() == 0) {
        return true; // End point edge case
    }
    Vector2DD normal = Vector2DD(v2.y() - v1.y(), v1.x() - v2.x());
    double dotProduct1 = normal.dot(v1v3);
    double dotProduct2 = normal.dot(v1v0);
    if (std::abs(dotProduct1) < settings_.epsilon || std::abs(dotProduct2) < settings_.epsilon) {
        return true;
    }
    double sign = dotProduct1 > 0 ? 1.0 : -1.0;
    return dotProduct2 * sign >= 0;
}

template<typename T>
T mix(const T &a, const T &b, double w) {
    return (1.0 - w) * a + w * b;
}

void SubdivisionCurve::knotCurve(QVector<Vector2DD> &coords, QVector<Vector2DD> &norms, QVector<bool> &customNorms) {
    int n = int(netCoords_.size());
    coords.reserve(n);
    norms.reserve(n);
    customNorms.reserve(n);
    int idx = 0;
    for (int i = 0; i < n; i++) {
        const Vector2DD &v0 = netCoords_[getPrevIdx(i)];
        const Vector2DD &v1 = netCoords_[i];
        int nextIdx = getNextIdx(i);
        const Vector2DD &v2 = netCoords_[nextIdx];
        const Vector2DD &v3 = netCoords_[getNextIdx(nextIdx)];
        coords.append(v1);
        norms.append(netNormals_[i]);
        customNorms.append(customNormals_[i]);
        idx++;
        if (v0 == v1 || v2 == v3 || v1 == v2) {
            continue;
        }
        if (!areInSameHalfPlane(v0, v1, v2, v3)) {

            Vector2DD reflectVec = (v1 - v2).normalized();
            reflectVec = {-reflectVec.y(), reflectVec.x()};
            double angle1 = netNormals_[i].dot(reflectVec);
            double angle2 = netNormals_[nextIdx].dot(reflectVec);

            double ratio = 0.5;
            if (settings_.weightedKnotLocation) {
                angle1 = (v0 - v1).normalized().dot((v1 - v2).normalized());
                angle2 = (v3 - v2).normalized().dot((v2 - v1).normalized());

                double l1 = std::abs(std::acos(angle1));
                double l2 = std::abs(std::acos(angle2));
                if (settings_.gravitateSmallerAngles) {
                    ratio = l1 / (l1 + l2);
                } else {

                    ratio = l2 / (l1 + l2);
                }
            }

            Vector2DD midPoint = mix(v1, v2, ratio);
            Vector2DD incident = mix(netNormals_[i], netNormals_[nextIdx], ratio).normalized();
            incident *= -1;
            incident = angle1 > angle2 ? netNormals_[i] : netNormals_[nextIdx];

            Vector2DD knotNormal = incident - 2 * (incident.dot(reflectVec)) * reflectVec; // reflect
            knotNormal *= -1;
            knotNormal.normalize();

            knotNormal = (1 - settings_.knotTension) * knotNormal + settings_.knotTension * reflectVec;
            knotNormal.normalize();
            coords.append(midPoint);
            norms.append(knotNormal);
            customNorms.append(true);
            knotIndices.insert(idx);
            idx++;
        }
    }
}

void SubdivisionCurve::applySubdivision() {
    netCoords_ = curveCoords_;
    netNormals_ = curveNormals_;
    customNormals_.resize(netCoords_.size());
    subdivisionLevel_ = 0;
}

void SubdivisionCurve::insertKnots() {
    QVector<Vector2DD> coords;
    QVector<Vector2DD> norms;
    QVector<bool> customNorms;
    knotCurve(coords, norms, customNorms);

    netCoords_ = coords;
    netNormals_ = norms;
    customNormals_ = customNorms;

}

QVector<double> SubdivisionCurve::getStabilityVals() const {
    return stability;
}

void SubdivisionCurve::translate(const Vector2DD &translation) {
    for (auto &c: netCoords_) {
        c += translation;
    }
    reSubdivide();
}
