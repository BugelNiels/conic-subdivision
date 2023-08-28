#include "conicsubdivider.hpp"
#include "core/conics/conic.hpp"
#include "core/subdivisioncurve.hpp"

ConicSubdivider::ConicSubdivider(const Settings &settings) : settings_(settings) {}

void ConicSubdivider::subdivide(SubdivisionCurve *newCurve, int level) {
    curve_ = newCurve;
    const auto &netCoords = curve_->getNetCoords();
    const auto &netNorms = curve_->getNetNormals();
    if (int(netCoords.size()) == 0) {
        return;
    }
    if (settings_.convexitySplit) {
        std::vector<Vector2DD> coords;
        std::vector<Vector2DD> norms;
        std::vector<bool> customNorms;
        knotCurve(curve_, coords, norms, customNorms);
        subdivide(coords, norms, level);
    } else {
        subdivide(netCoords, netNorms, level);
    }
}

void ConicSubdivider::subdivide(const std::vector<Vector2DD> &points,
                                const std::vector<Vector2DD> &normals,
                                int level) {
    // base case
    if (level == 0) {
        curve_->curveCoords_ = points;
        curve_->curveNormals_ = normals;
        return;
    }
    int n = int(points.size()) * 2 - 1;
    if (curve_->isClosed()) {
        n += 1;
    }
    std::vector<Vector2DD> newPoints(n);
    std::vector<Vector2DD> newNormals(n);

    // set vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
    }
    // set new points
    for (int i = 1; i < n; i += 2) {
        vertexPoint(points, normals, i, newPoints, newNormals);
    }
    if (settings_.recalculateNormals) {
        newNormals = curve_->calcNormals(newPoints);
    }
    std::set<int> newKnotIndices;
    for (const auto &knotIdx: curve_->knotIndices_) {
        newKnotIndices.insert(knotIdx * 2);
    }
    curve_->knotIndices_ = newKnotIndices;
    subdivide(newPoints, newNormals, level - 1);
}

void ConicSubdivider::vertexPoint(const std::vector<Vector2DD> &points,
                                  const std::vector<Vector2DD> &normals,
                                  int i,
                                  std::vector<Vector2DD> &newPoints,
                                  std::vector<Vector2DD> &newNormals) const {
    int n = newPoints.size();
    std::vector<PatchPoint> patchPoints;
    extractPatch(points, normals, i / 2, patchPoints, settings_.patchSize);

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
    Conic conic(patchPoints, settings_);
    Vector2DD sampledPoint;
    Vector2DD sampledNormal;
    bool valid = conic.sample(origin, dir, sampledPoint, sampledNormal);

    if (!valid) {
        if (settings_.dynamicPatchSize) {
            int patchSize = settings_.patchSize + 1;
            int oldPatchSize = patchPoints.size();
            while (!valid) {
                patchPoints.clear();
                extractPatch(points, normals, i / 2, patchPoints, patchSize);
                if (patchSize > 10 || patchPoints.size() == oldPatchSize) {
                    sampledPoint = origin;
                    sampledNormal = dir;
                    break;
                }
                oldPatchSize = patchPoints.size();
                Conic conic2(patchPoints, settings_);
                valid = conic2.sample(origin, dir, sampledPoint, sampledNormal);
                patchSize++;
            }
        } else {
            sampledPoint = origin;
            sampledNormal = dir;
        }
    }

    newPoints[i] = sampledPoint;
    newNormals[i] = sampledNormal;
}

bool arePointsCollinear(const Vector2DD &p1, const Vector2DD &p2, const Vector2DD &p3) {
    return p1.x() * (p2.y() - p3.y()) + p2.x() * (p3.y() - p1.y()) + p3.x() * (p1.y() - p2.y()) <
           0.0001;
}

void ConicSubdivider::extractPatch(const std::vector<Vector2DD> &points,
                                   const std::vector<Vector2DD> &normals,
                                   int pIdx,
                                   std::vector<PatchPoint> &patchPoints,
                                   int patchSize) const {
    int size = int(points.size());

    if (curve_->isClosed()) {
        // Left middle
        int leftMiddleIdx = pIdx;
        bool leftKnotPoint = curve_->knotIndices_.count(leftMiddleIdx) > 0;
        patchPoints.push_back({points[leftMiddleIdx],
                               normals[leftMiddleIdx],
                               settings_.middlePointWeight,
                               settings_.middleNormalWeight});

        // Right middle
        int rightMiddleIdx = (pIdx + 1) % size;
        bool rightKnotPoint = curve_->knotIndices_.count(rightMiddleIdx) > 0;
        patchPoints.push_back({points[rightMiddleIdx],
                               normals[rightMiddleIdx],
                               settings_.middlePointWeight,
                               settings_.middleNormalWeight});

        if (!leftKnotPoint) {
            int leftOuterIdx = (leftMiddleIdx - 1 + size) % size;
            for (int i = 1; i < patchSize; ++i) {
                int idx = (leftMiddleIdx - i + size) % size;
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[leftOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back({points[idx],
                                       normals[idx],
                                       settings_.outerPointWeight,
                                       settings_.outerNormalWeight});
            }
        }

        if (!rightKnotPoint) {
            int rightOuterIdx = (rightMiddleIdx + 1) % size;
            for (int i = 1; i < patchSize; ++i) {
                int idx = (rightMiddleIdx + i) % size;
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[rightOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back({points[idx],
                                       normals[idx],
                                       settings_.outerPointWeight,
                                       settings_.outerNormalWeight});
            }
        }
    } else {
        // Left middle
        int leftMiddleIdx = pIdx;
        bool leftKnotPoint = curve_->knotIndices_.count(leftMiddleIdx) > 0;
        patchPoints.push_back({points[leftMiddleIdx],
                               normals[leftMiddleIdx],
                               settings_.middlePointWeight,
                               settings_.middleNormalWeight});

        // Right middle
        int rightMiddleIdx = (pIdx + 1);
        bool rightKnotPoint = curve_->knotIndices_.count(rightMiddleIdx) > 0;
        patchPoints.push_back({points[rightMiddleIdx],
                               normals[rightMiddleIdx],
                               settings_.middlePointWeight,
                               settings_.middleNormalWeight});

        if (!leftKnotPoint) {
            int leftOuterIdx = leftMiddleIdx - 1;
            for (int i = 1; i < patchSize; ++i) {
                int idx = leftMiddleIdx - i;
                if (idx < 0 || leftOuterIdx < 0) {
                    break;
                }
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[leftOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back({points[idx],
                                       normals[idx],
                                       settings_.outerPointWeight,
                                       settings_.outerNormalWeight});
            }
        }

        if (!rightKnotPoint) {
            int rightOuterIdx = rightMiddleIdx + 1;
            for (int i = 1; i < patchSize; ++i) {
                int idx = rightMiddleIdx + i;
                if (idx >= size || rightOuterIdx >= size) {
                    break;
                }
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[rightOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back({points[idx],
                                       normals[idx],
                                       settings_.outerPointWeight,
                                       settings_.outerNormalWeight});
            }
        }
    }
}

bool ConicSubdivider::areInSameHalfPlane(const Vector2DD &v0,
                                         const Vector2DD &v1,
                                         const Vector2DD &v2,
                                         const Vector2DD &v3) const {
    Vector2DD v1v3 = v3 - v1;
    Vector2DD v1v0 = v0 - v1;
    if (v1v0.squaredNorm() == 0.0 || v1v3.squaredNorm() == 0) {
        return true; // End point edge case
    }
    Vector2DD normal = Vector2DD(v2.y() - v1.y(), v1.x() - v2.x());
    long double dotProduct1 = normal.dot(v1v3);
    long double dotProduct2 = normal.dot(v1v0);
    if (std::abs(dotProduct1) < settings_.epsilon || std::abs(dotProduct2) < settings_.epsilon) {
        return true;
    }
    double sign = dotProduct1 > 0 ? 1.0 : -1.0;
    return dotProduct2 * sign >= 0;
}

template<typename T>
T mix(const T &a, const T &b, long double w) {
    return (1.0 - w) * a + w * b;
}

Vector2DD rotate(const Vector2DD &v, double angle) {
    long double x_new = v.x() * std::cos(angle) - v.y() * std::sin(angle);
    long double y_new = v.x() * std::sin(angle) + v.y() * std::cos(angle);
    return Vector2DD(x_new, y_new);
}

Vector2DD clamp(const Vector2DD &vec, const Vector2DD &a, const Vector2DD &b) {
    long double clampedX = std::max(std::min(vec.x(), std::max(a.x(), b.x())),
                                    std::min(a.x(), b.x()));
    long double clampedY = std::max(std::min(vec.y(), std::max(a.y(), b.y())),
                                    std::min(a.y(), b.y()));
    return Vector2DD(clampedX, clampedY);
}

void ConicSubdivider::knotCurve(SubdivisionCurve *curve,
                                std::vector<Vector2DD> &coords,
                                std::vector<Vector2DD> &norms,
                                std::vector<bool> &customNorms) {

    curve->knotIndices_.clear();
    int n = int(curve->netCoords_.size());
    coords.reserve(n);
    norms.reserve(n);
    customNorms.reserve(n);
    int idx = 0;
    for (int i = 0; i < n; i++) {
        const Vector2DD &v0 = curve->netCoords_[curve->getPrevIdx(i)];
        const Vector2DD &v1 = curve->netCoords_[i];
        int nextIdx = curve->getNextIdx(i);
        const Vector2DD &v2 = curve->netCoords_[nextIdx];
        const Vector2DD &v3 = curve->netCoords_[curve->getNextIdx(nextIdx)];
        coords.emplace_back(v1);
        norms.emplace_back(curve->netNormals_[i]);
        customNorms.emplace_back(curve->customNormals_[i]);
        idx++;
        if (v0 == v1 || v2 == v3 || v1 == v2) {
            continue;
        }
        if (!areInSameHalfPlane(v0, v1, v2, v3)) {

            Vector2DD v1v0 = (v1 - v0).normalized();
            Vector2DD v1v2 = (v1 - v2).normalized();
            Vector2DD v3v2 = (v3 - v2).normalized();

            Vector2DD orthogonalV1V2 = {-v1v2.y(), v1v2.x()};

            long double ratio = 0.5;
            if (settings_.weightedKnotLocation) {
                long double dot1 = v1v0.dot(orthogonalV1V2);
                long double dot2 = v3v2.dot(orthogonalV1V2);
                dot1 = (v0 - v1).normalized().dot((v1 - v2).normalized());
                dot2 = (v3 - v2).normalized().dot((v2 - v1).normalized());

                long double l1 = std::abs(std::acos(dot1));
                long double l2 = std::abs(std::acos(dot2));
                if (settings_.gravitateSmallerAngles) {
                    ratio = l1 / (l1 + l2);
                } else {

                    ratio = l2 / (l1 + l2);
                }
            }

            Vector2DD midPoint = mix(v1, v2, ratio);

            Vector2DD diagonal = {1, 1};
            diagonal.normalize();
            Vector2DD inflNormalLeft;
            //            Vector2DD normLeft = curve->netNormals_[i].normalized();
            Vector2DD normLeft = ((v0 - v1).normalized() + (v2 - v1).normalized()).normalized();

            double leftAngle;
            long double smallestOrthoAngleLeft = std::min(
                    std::abs(std::acos(normLeft.dot(orthogonalV1V2))),
                    std::abs(std::acos(normLeft.dot(-orthogonalV1V2))));
            long double smallestStraightAngleLeft = std::min(
                    std::abs(std::acos(normLeft.dot(v1v2))),
                    std::abs(std::acos(normLeft.dot(-v1v2))));
            if (smallestStraightAngleLeft > smallestOrthoAngleLeft) {
                // The normal makes a sharper angle with the orthogonal vector
                // Take the normal and reflect it around the orthogonal vector to obtain the left inflection normal
                inflNormalLeft = normLeft - 2 * (normLeft.dot(orthogonalV1V2)) * orthogonalV1V2;
                inflNormalLeft *= -1;
                inflNormalLeft.normalize();
                leftAngle = smallestOrthoAngleLeft;
            } else {
                // The normal makes a sharper angle with the edge itself. Simply rotate by 90 degrees
                inflNormalLeft = {-normLeft.y(), normLeft.x()};
                leftAngle = smallestStraightAngleLeft;
            }

            Vector2DD inflNormalRight;
            //            Vector2DD normRight = curve->netNormals_[nextIdx].normalized();
            Vector2DD normRight = ((v3 - v2).normalized() + (v1 - v2).normalized()).normalized();
            double rightAngle;

            long double smallestOrthoAngleRight = std::min(
                    std::abs(std::acos(normRight.dot(orthogonalV1V2))),
                    std::abs(std::acos(normRight.dot(-orthogonalV1V2))));
            long double smallestStraightAngleRight = std::min(
                    std::abs(std::acos(normRight.dot(v1v2))),
                    std::abs(std::acos(normRight.dot(-v1v2))));
            if (smallestStraightAngleRight > smallestOrthoAngleRight) {
                // The normal makes a sharper angle with the orthogonal vector
                // Take the normal and reflect it around the orthogonal vector to obtain the left inflection normal
                inflNormalRight = normRight - 2 * (normRight.dot(orthogonalV1V2)) * orthogonalV1V2;
                inflNormalRight *= -1;
                inflNormalRight.normalize();
                rightAngle = smallestOrthoAngleRight;
            } else {
                // The normal makes a sharper angle with the edge itself. Simply rotate by 90 degrees
                inflNormalRight = {normRight.y(), -normRight.x()};
                rightAngle = smallestStraightAngleRight;
            }
            // Take the minimum?
            Vector2DD knotNormal = leftAngle < rightAngle ? inflNormalLeft : inflNormalRight;
            knotNormal.normalize();

            // save
            coords.emplace_back(midPoint);
            norms.emplace_back(knotNormal);
            customNorms.emplace_back(true);
            curve->knotIndices_.insert(idx);
            idx++;
        }
    }
}