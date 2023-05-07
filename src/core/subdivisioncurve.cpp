#include "subdivisioncurve.hpp"

#include "src/core/conics/conicfitter.hpp"

#include "settings.hpp"
#include "core/conics/conic.hpp"

#define MIN(A, B) (A) < (B) ? (A) : (B)
#define MAX(A, B) (A) > (B) ? (A) : (B)

/**
 * @brief SubdivisionCurve::SubdivisionCurve Creates a new subdivision curve.
 */
SubdivisionCurve::SubdivisionCurve() : subdivisionLevel_(0) {}


SubdivisionCurve::SubdivisionCurve(Settings *settings, QVector<QVector2D> coords) : settings_(settings),
                                                                                    netCoords_(coords),
                                                                                    subdivisionLevel_(0) {
    netNormals_ = calcNormals(netCoords_);
}


SubdivisionCurve::SubdivisionCurve(Settings *settings, QVector<QVector2D> coords, QVector<QVector2D> normals)
        : settings_(settings), netCoords_(coords),
          netNormals_(normals),
          subdivisionLevel_(0) {

}

QVector2D calcNormal(const QVector2D &a, const QVector2D &b,
                     const QVector2D &c) {
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
    if (true) {
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
    if (settings_->closed) {
        prevIdx = (i - 1 + n) % n;
        nextIdx = (i + 1) % n;
    } else {
        prevIdx = MAX(0, i - 1);
        nextIdx = MIN(i + 1, n - 1);
    }
    QVector2D a = coords[prevIdx];
    QVector2D b = coords[i];
    QVector2D c = coords[nextIdx];
    if (settings_->circleNormals) {
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

            QVector2D check = calcNormal(a, b, c);
            if (QVector2D::dotProduct(check, normals[i]) < 0) {
                normals[i] *= -1;
            }
        }
    } else {
        normals[i] = calcNormal(a, b, c);
    }
}

/**
 * @brief SubdivisionCurve::addPoint Adds a point to the control net.
 * @param p The point to add to the control net.
 */
void SubdivisionCurve::addPoint(QVector2D p) {
    netCoords_.append(p);
    netNormals_.append(QVector2D());
    calcNormalAtIndex(netCoords_, netNormals_, netNormals_.size() - 1);
    reSubdivide();
}

/**
 * @brief SubdivisionCurve::setVertexPosition Changes the position of a point in
 * the control net.
 * @param idx The index of the point to update the position of.
 * @param p The new position of the point.
 */
void SubdivisionCurve::setVertexPosition(int idx, QVector2D p) {
    netCoords_[idx] = p;
    reSubdivide();
}

void SubdivisionCurve::setNormalPosition(int idx, QVector2D p) {
    netNormals_[idx] = p - netCoords_[idx];
    netNormals_[idx].normalize();
    reSubdivide();
}

/**
 * @brief SubdivisionCurve::removePoint Removes a point from the control net.
 * @param idx The index of the point to remove.
 */
void SubdivisionCurve::removePoint(int idx) {
    netCoords_.remove(idx);
    netNormals_.remove(idx);
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
    float currentDist, minDist = 4;

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

// Returns index of the point normal thingie
int SubdivisionCurve::findClosestNormal(const QVector2D &p, const float maxDist) {
    int ptIndex = -1;
    float currentDist, minDist = 4;

    // TODO: handle subdivided normals
    for (int k = 0; k < netCoords_.size(); k++) {
        QVector2D normPos = netCoords_[k] + settings_->normalLength * netNormals_[k];
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
    if (level > subdivisionLevel_ && subdivisionLevel_ > 0) {
        // don't start subdividing from the start
        // if the previous subdivision level was 3 and the new one is 4, then we
        // only need to do a single subdividision step, instead of starting from
        // scratch and doing 4 steps.
        subdivide(curveCoords_, curveNormals_, level - subdivisionLevel_);
    } else {
        subdivide(netCoords_, netNormals_, level);
    }
    subdivisionLevel_ = level;
}

/**
 * @brief SubdivisionCurve::subdivide Recursive subdivision function. Subdivides
 * the provided points recursively a number of times according to the
 * subdivision mask.
 * @param points The points to be subdivided
 * @param level The number of times the points should be subdivided
 */
void SubdivisionCurve::subdivide(const QVector<QVector2D> &points,
                                 const QVector<QVector2D> &normals, int level) {
    // base case
    if (level == 0) {
        curveCoords_ = points;
        curveNormals_ = normals;
        return;
    }
    int n = points.size() * 2 - 1;
    if (settings_->closed) {
        n += 1;
    }
    QVector<QVector2D> newPoints(n);
    QVector<QVector2D> newNormals(n);

    // set vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
    }
    // set new points
    int patchSize = 6;
    QVector<int> indices(patchSize);
    for (int i = 1; i < n; i += 2) {
        QVector<QVector2D> patchCoords;
        QVector<QVector2D> patchNormals;

        int pIdx = i / 2;

        int size = points.size();

        indices.clear();
        indices.append(pIdx);
        // p_2-p_1-p0-p1-p2-p3
        if (settings_->closed) {
            indices.append((pIdx + 1) % size);
            indices.append((pIdx + 2) % size);
            indices.append((pIdx - 1 + size) % size);
            indices.append((pIdx + 3) % size);
            indices.append((pIdx - 2 + size) % size);
        } else {
            if (pIdx + 1 < points.size()) {
                indices.append(pIdx + 1);
                if (pIdx + 2 < points.size()) {
                    indices.append(pIdx + 2);
                }
            }
            if (pIdx - 1 >= 0) {
                indices.append(pIdx - 1);
                if (pIdx - 2 >= 0) {
                    indices.append(pIdx - 2);
                }
            }

            if (pIdx + 3 < points.size()) {
                indices.append(pIdx + 3);
            }
        }

        for (int index: indices) {
            patchCoords.append(points[index]);
            patchNormals.append(normals[index]);
        }

        Conic conic(patchCoords, patchNormals, *settings_);

        int prevIdx = (i - 1 + n) % n;
        int nextIdx = (i + 1) % n;

        const QVector2D origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0f;
        QVector2D dir;
        if (settings_->edgeTangentSample) {
            dir = newPoints[prevIdx] - newPoints[nextIdx];
            dir.normalize();
            dir = {-dir.y(), dir.x()};
        } else {
            dir = (newNormals[prevIdx] + newNormals[nextIdx]).normalized();
        }
        QVector2D sampledPoint;
        QVector2D sampledNormal;
        const bool valid = conic.sample(origin, dir, sampledPoint, sampledNormal);

        if (!valid) {
            sampledPoint = origin;
            sampledNormal = dir;
        }

        newPoints[i] = sampledPoint;
        newNormals[i] = sampledNormal;
    }
    if (settings_->recalculateNormals) {
        newNormals = calcNormals(newPoints);
    }
    subdivide(newPoints, newNormals, level - 1);
}

void SubdivisionCurve::flipNormals() {
    for (auto &n: netNormals_) {
        n *= -1;
    }
}

void SubdivisionCurve::recalculateNormals() {
    netNormals_ = calcNormals(netCoords_);
}
