#include "subdivisioncurve.hpp"

#include <utility>

#include "src/core/conics/conicfitter.hpp"

#include "settings.hpp"
#include "core/conics/conic.hpp"

#define MIN(A, B) (A) < (B) ? (A) : (B)
#define MAX(A, B) (A) > (B) ? (A) : (B)
#define EPSILON 0.00000001


SubdivisionCurve::SubdivisionCurve(const Settings &settings)
        : settings_(settings),
          closed_(true) {

}


SubdivisionCurve::SubdivisionCurve(const Settings &settings, QVector<QVector2D> coords, bool closed)
        : settings_(settings),
          closed_(closed),
          netCoords_(std::move(coords)) {
    netNormals_ = calcNormals(netCoords_);
    customNormals_.resize(netNormals_.size());
    customNormals_.fill(false);
}


SubdivisionCurve::SubdivisionCurve(const Settings &settings, QVector<QVector2D> coords, QVector<QVector2D> normals,
                                   bool closed)
        : settings_(settings),
          closed_(closed),
          netCoords_(std::move(coords)),
          netNormals_(std::move(normals)) {
    customNormals_.resize(netNormals_.size());
    customNormals_.fill(false);
}

QVector2D calcNormal(const QVector2D &a, const QVector2D &b,
                     const QVector2D &c, bool areaWeighted) {
    if (a == b) {
        QVector2D normal = c - b;
        normal.setX(normal.x() * -1);
        return QVector2D(normal.y(), normal.x()).normalized();
    }
    if (b == c) {
        QVector2D normal = b - a;
        normal.setX(normal.x() * -1);
        return QVector2D(normal.y(), normal.x()).normalized();
    }
    QVector2D t1 = (a - b);
    t1 = {-t1.y(), t1.x()};
    QVector2D t2 = (b - c);
    t2 = {-t2.y(), t2.x()};
    if (!areaWeighted) {
        t1.normalize();
        t2.normalize();
    }

    return (t1 + t2).normalized();
}

QVector<QVector2D> SubdivisionCurve::calcNormals(
        const QVector<QVector2D> &coords) const {
    QVector<QVector2D> normals;
    int n = coords.size();
    normals.resize(n);
    for (int i = 0; i < n; i++) {
        calcNormalAtIndex(coords, normals, i);
    }

    return normals;
}

void SubdivisionCurve::calcNormalAtIndex(const QVector<QVector2D> &coords, QVector<QVector2D> &normals, int i) const {
    int n = normals.size();
    int nextIdx, prevIdx;
    if (closed_) {
        prevIdx = (i - 1 + n) % n;
        nextIdx = (i + 1) % n;
    } else {
        prevIdx = MAX(0, i - 1);
        nextIdx = MIN(i + 1, n - 1);
    }
    QVector2D a = coords[prevIdx];
    QVector2D b = coords[i];
    QVector2D c = coords[nextIdx];
    if (settings_.circleNormals) {
        if (a == b) {
            QVector2D normal = c - b;
            normal.setX(normal.x() * -1);
            normals[i] = QVector2D(normal.y(), normal.x()).normalized();
        } else if (b == c) {
            QVector2D normal = b - a;
            normal.setX(normal.x() * -1);
            normals[i] = QVector2D(normal.y(), normal.x()).normalized();
        } else {
            float d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) +
                           c.x() * (a.y() - b.y()));
            float ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                        (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                        (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                       d;
            float uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                        (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                        (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                       d;
            QVector2D oscCircleCenter = QVector2D(ux, uy);
            normals[i] = (oscCircleCenter - b).normalized();

            QVector2D check = calcNormal(a, b, c, settings_.areaWeightedNormals);
            if (QVector2D::dotProduct(check, normals[i]) < 0) {
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
int SubdivisionCurve::addPoint(QVector2D p) {
    int idx = findInsertIdx(p);
    netCoords_.insert(idx, p);
    netNormals_.insert(idx, QVector2D());
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
void SubdivisionCurve::setVertexPosition(int idx, QVector2D p) {
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

void SubdivisionCurve::setNormalPosition(int idx, QVector2D p) {
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
int SubdivisionCurve::findClosestVertex(const QVector2D &p, const float maxDist) {
    int ptIndex = -1;
    float currentDist, minDist = std::numeric_limits<float>::infinity();

    for (int k = 0; k < netCoords_.size(); k++) {
        currentDist = netCoords_[k].distanceToPoint(p);
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
    int n = netCoords_.size();
    return closed_ ? (idx + 1) % n : MIN(idx + 1, n - 1);
}

int SubdivisionCurve::getPrevIdx(int idx) {
    int n = netCoords_.size();
    return closed_ ? (idx - 1 + n) % n : MAX(idx - 1, 0);
}

float distanceToEdge(const QVector2D &A, const QVector2D &B, const QVector2D &P) {
    QVector2D AB = B - A;
    QVector2D AP = P - A;
    float AB_len2 = AB.lengthSquared();
    float t = QVector2D::dotProduct(AP, AB) / AB_len2;
    t = qMax(0.0f, qMin(1.0f, t)); // clamp to [0, 1]
    QVector2D Q = A + t * AB;
    return std::hypot(P.x() - Q.x(), P.y() - Q.y());
}

int SubdivisionCurve::findInsertIdx(const QVector2D &p) {
    if (netCoords_.empty()) {
        return 0;
    }
    int ptIndex = -1;
    float currentDist, minDist = std::numeric_limits<float>::infinity();

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
int SubdivisionCurve::findClosestNormal(const QVector2D &p, const float maxDist) {
    int ptIndex = -1;
    float currentDist, minDist = 4;

    for (int k = 0; k < netCoords_.size(); k++) {
        QVector2D normPos = netCoords_[k] + settings_.normalLength * netNormals_[k];
        currentDist = normPos.distanceToPoint(p);
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
    if (netCoords_.size() == 0) {
        return;
    }
    if (settings_.convexitySplit) {

        QVector<QVector2D> coords;
        QVector<QVector2D> norms;
        QVector<bool> customNorms;
        knotCurve(coords, norms, customNorms);
        QVector<float> stabilities;
        stabilities.resize(coords.size());
        subdivide(coords, norms, stabilities, level);
    } else {
        stability.resize(netCoords_.size());
        subdivide(netCoords_, netNormals_, stability, level);
    }

    subdivisionLevel_ = level;
}

bool arePointsCollinear(const QVector2D &p1, const QVector2D &p2, const QVector2D &p3) {
    return p1.x() * (p2.y() - p3.y()) + p2.x() * (p3.y() - p1.y()) + p3.x() * (p1.y() - p2.y()) == 0.0;
}

/**
 * @brief SubdivisionCurve::subdivide Recursive subdivision function. Subdivides
 * the provided points recursively a number of times according to the
 * subdivision mask.
 * @param points The points to be subdivided
 * @param level The number of times the points should be subdivided
 */
void SubdivisionCurve::subdivide(const QVector<QVector2D> &points,
                                 const QVector<QVector2D> &normals,
                                 const QVector<float> &stabilities, int level) {
    // base case
    if (level == 0) {
        curveCoords_ = points;
        curveNormals_ = normals;
        stability = stabilities;
        return;
    }
    int n = points.size() * 2 - 1;
    if (closed_) {
        n += 1;
    }
    QVector<QVector2D> newPoints(n);
    QVector<QVector2D> newNormals(n);
    QVector<float> newStabilities(n);

    // set vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
        newStabilities[i] = stabilities[i / 2];
    }
    // set new points
    int patchSize = 6;
    QVector<int> indices(patchSize);
    for (int i = 1; i < n; i += 2) {
        QVector<QVector2D> patchCoords;
        QVector<QVector2D> patchNormals;

        extractPatch(points, normals, indices, i, patchCoords, patchNormals);

        int prevIdx = (i - 1 + n) % n;
        int nextIdx = (i + 1) % n;

        const QVector2D origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0f;
        QVector2D dir;
        if (settings_.edgeTangentSample) {
            dir = newPoints[prevIdx] - newPoints[nextIdx];
            dir.normalize();
            dir = {-dir.y(), dir.x()};
        } else {
            dir = (newNormals[prevIdx] + newNormals[nextIdx]).normalized();
        }
        Conic conic(patchCoords, patchNormals, settings_);
        QVector2D sampledPoint;
        QVector2D sampledNormal;

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

void SubdivisionCurve::extractPatch(const QVector<QVector2D> &points, const QVector<QVector2D> &normals,
                                    QVector<int> &indices, int i, QVector<QVector2D> &patchCoords,
                                    QVector<QVector2D> &patchNormals) const {
    int pIdx = i / 2;

    int size = points.size();

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
        }
        if (!endKnotPoint) {
            indices.append((pIdx + 2) % size);
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

void SubdivisionCurve::flipNormals() {
    for (auto &n: netNormals_) {
        n *= -1;
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
        recalculateNormal(customNormals_.size() - 1);
    }
    reSubdivide();
}

bool areInSameHalfPlane(const QVector2D &v0, const QVector2D &v1, const QVector2D &v2, const QVector2D &v3) {
    QVector2D v1v3 = v3 - v1;
    QVector2D v1v0 = v0 - v1;
    if (v1v0.lengthSquared() == 0.0 || v1v3.lengthSquared() == 0) {
        return true; // End point edge case
    }
    QVector2D normal = QVector2D(v2.y() - v1.y(), v1.x() - v2.x());
    float dotProduct1 = QVector2D::dotProduct(normal, v1v3);
    float dotProduct2 = QVector2D::dotProduct(normal, v1v0);
    if (std::abs(dotProduct1) < EPSILON || std::abs(dotProduct2) < EPSILON) {
        return true;
    }
    float sign = dotProduct1 > 0 ? 1.0f : -1.0f;
    return dotProduct2 * sign >= 0;
}

template<typename T>
T mix(const T &a, const T &b, float w) {
    return (1.0f - w) * a + w * b;
}

void SubdivisionCurve::knotCurve(QVector<QVector2D> &coords, QVector<QVector2D> &norms, QVector<bool> &customNorms) {
    int n = netCoords_.size();
    coords.reserve(n);
    norms.reserve(n);
    customNorms.reserve(n);
    int idx = 0;
    for (int i = 0; i < n; i++) {
        const QVector2D &v0 = netCoords_[getPrevIdx(i)];
        const QVector2D &v1 = netCoords_[i];
        int nextIdx = getNextIdx(i);
        const QVector2D &v2 = netCoords_[nextIdx];
        const QVector2D &v3 = netCoords_[getNextIdx(nextIdx)];
        coords.append(v1);
        norms.append(netNormals_[i]);
        customNorms.append(customNormals_[i]);
        idx++;
        if (v0 == v1 || v2 == v3 || v1 == v2) {
            continue;
        }
        if (!areInSameHalfPlane(v0, v1, v2, v3)) {

            QVector2D reflectVec = (v1 - v2).normalized();
            reflectVec = {-reflectVec.y(), reflectVec.x()};
            float angle1 = QVector2D::dotProduct(netNormals_[i], reflectVec);
            float angle2 = QVector2D::dotProduct(netNormals_[nextIdx], reflectVec);

            float ratio = 0.5f;
            if (settings_.weightedKnotLocation) {
//                float l1 = (v0 - v1).length();
//                float l2 = (v3 - v2).length();
                angle1 = QVector2D::dotProduct((v0 - v1).normalized(), (v1 - v2).normalized());
                angle2 = QVector2D::dotProduct((v3 - v2).normalized(), (v2 - v1).normalized());

                float l1 = std::abs(std::acos(angle1));
                float l2 = std::abs(std::acos(angle2));
                if (settings_.gravitateSmallerAngles) {
                    ratio = l1 / (l1 + l2);
                } else {

                    ratio = l2 / (l1 + l2);
                }
            }

            QVector2D midPoint = mix(v1, v2, ratio);
            QVector2D incident = mix(netNormals_[i], netNormals_[nextIdx], ratio).normalized();
            incident *= -1;
            incident = angle1 > angle2 ? netNormals_[i] : netNormals_[nextIdx];

            QVector2D knotNormal = incident - 2 * (QVector2D::dotProduct(incident, reflectVec)) * reflectVec; // reflect
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
    QVector<QVector2D> coords;
    QVector<QVector2D> norms;
    QVector<bool> customNorms;
    knotCurve(coords, norms, customNorms);

    netCoords_ = coords;
    netNormals_ = norms;
    customNormals_ = customNorms;

}

QVector<float> SubdivisionCurve::getStabilityVals() const {
    return stability;
}

void SubdivisionCurve::translate(const QVector2D &translation) {
    for (auto &c: netCoords_) {
        c += translation;
    }
    reSubdivide();
}
