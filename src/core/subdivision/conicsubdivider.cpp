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
        insertInflPoints(curve_, coords, norms, customNorms);
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

    // set old vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
    }
    // set new edge points
    for (int i = 1; i < n; i += 2) {
        edgePoint(points, normals, i, newPoints, newNormals);
    }
    // Update the indices of the inflection points
    std::set<int> newInflPointIndices;
    for (const auto &inflIdx: curve_->inflPointsIndices_) {
        newInflPointIndices.insert(inflIdx * 2);
    }
    curve_->inflPointsIndices_ = newInflPointIndices;
    subdivide(newPoints, newNormals, level - 1);
}

void ConicSubdivider::edgePoint(const std::vector<Vector2DD> &points,
                                const std::vector<Vector2DD> &normals,
                                int i,
                                std::vector<Vector2DD> &newPoints,
                                std::vector<Vector2DD> &newNormals) const {
    int n = newPoints.size();
    std::vector<PatchPoint> patchPoints = extractPatch(points, normals, i / 2, settings_.patchSize);

    int prevIdx = (i - 1 + n) % n;
    int nextIdx = (i + 1) % n;

    const Vector2DD origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0;
    Vector2DD dir = newPoints[prevIdx] - newPoints[nextIdx];
    dir = {-dir.y(), dir.x()};
    // Note that dir is not normalized!
    Conic conic(patchPoints, settings_);
    Vector2DD sampledPoint;
    Vector2DD sampledNormal;
    bool valid = conic.sample(origin, dir, sampledPoint, sampledNormal);
    if (!valid) {
        if (settings_.dynamicPatchSize) {
            // Keep growing the patch size until a solution is found
            // (or until the patch size cannot grow any further without violating the convexity property)
            int patchSize = settings_.patchSize + 1;
            int oldPatchSize = patchPoints.size();
            while (!valid) {
                patchPoints = extractPatch(points, normals, i / 2, patchSize);
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

std::vector<PatchPoint> ConicSubdivider::extractPatch(const std::vector<Vector2DD> &points,
                                   const std::vector<Vector2DD> &normals,
                                   int pIdx,
                                   int maxPatchSize) const {

    std::vector<PatchPoint> patchPoints;
    patchPoints.reserve(4);
    int size = int(points.size());
    // Left middle
    int leftMiddleIdx = pIdx;
    bool leftInflPoint = curve_->inflPointsIndices_.count(leftMiddleIdx) > 0;
    patchPoints.push_back({points[leftMiddleIdx],
                           normals[leftMiddleIdx],
                           settings_.middlePointWeight,
                           settings_.middleNormalWeight});

    // Right middle
    int rightMiddleIdx = (pIdx + 1) % size;
    bool rightInflPoint = curve_->inflPointsIndices_.count(rightMiddleIdx) > 0;
    patchPoints.push_back({points[rightMiddleIdx],
                           normals[rightMiddleIdx],
                           settings_.middlePointWeight,
                           settings_.middleNormalWeight});

    if (curve_->isClosed()) {
        if (!leftInflPoint) {
            int leftOuterIdx = (leftMiddleIdx - 1 + size) % size;
            for (int i = 1; i < maxPatchSize; ++i) {
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
        if (!rightInflPoint) {
            int rightOuterIdx = (rightMiddleIdx + 1) % size;
            for (int i = 1; i < maxPatchSize; ++i) {
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
        if (!leftInflPoint) {
            int leftOuterIdx = leftMiddleIdx - 1;
            for (int i = 1; i < maxPatchSize; ++i) {
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
        if (!rightInflPoint) {
            int rightOuterIdx = rightMiddleIdx + 1;
            for (int i = 1; i < maxPatchSize; ++i) {
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
    return patchPoints;
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
        return true; // curve is flat
    }
    double sign = dotProduct1 > 0 ? 1.0 : -1.0;
    return dotProduct2 * sign >= 0;
}

void ConicSubdivider::insertInflPoints(SubdivisionCurve *curve,
                                       std::vector<Vector2DD> &coords,
                                       std::vector<Vector2DD> &norms,
                                       std::vector<bool> &customNorms) {
    // Setup
    curve->inflPointsIndices_.clear();
    int n = int(curve->netCoords_.size());
    coords.reserve(n);
    norms.reserve(n);
    customNorms.reserve(n);
    int idx = 0;
    // For all points
    for (int i = 0; i < n; i++) {
        int nextIdx = curve->getNextIdx(i);
        const Vector2DD &v0 = curve->netCoords_[curve->getPrevIdx(i)];
        const Vector2DD &v1 = curve->netCoords_[i];
        const Vector2DD &v2 = curve->netCoords_[nextIdx];
        const Vector2DD &v3 = curve->netCoords_[curve->getNextIdx(nextIdx)];

        // Insert original points and normals
        coords.emplace_back(v1);
        norms.emplace_back(curve->netNormals_[i]);
        customNorms.emplace_back(curve->customNormals_[i]);
        idx++;
        // For non-closed curves or when there are not enough points (<= 2)
        if (v0 == v1 || v2 == v3 || v1 == v2) {
            continue;
        }

        // Insert an inflection point
        if (!areInSameHalfPlane(v0, v1, v2, v3)) {
            Vector2DD v1v0 = (v1 - v0).normalized();
            Vector2DD v1v2 = (v1 - v2).normalized();
            Vector2DD v3v2 = (v3 - v2).normalized();

            Vector2DD orthogonalV1V2 = {-v1v2.y(), v1v2.x()};

            long double ratio = 0.5;
            if (settings_.weightedInflPointLocation) {
                long double dot1 = (v0 - v1).normalized().dot((v1 - v2).normalized());
                long double dot2 = (v3 - v2).normalized().dot((v2 - v1).normalized());
                long double l1 = std::abs(std::acos(dot1));
                long double l2 = std::abs(std::acos(dot2));
                if (settings_.gravitateSmallerAngles) {
                    ratio = l1 / (l1 + l2);
                } else {
                    ratio = l2 / (l1 + l2);
                }
            }

            //            Vector2DD normLeft = curve->netNormals_[i].normalized();
            Vector2DD normLeft = ((v0 - v1).normalized() + (v2 - v1).normalized()).normalized();
            auto leftNorm = inflNormal(normLeft, v1v2, orthogonalV1V2);
            Vector2DD inflNormalLeft = leftNorm.first;
            long double leftAngle = leftNorm.second;

            //            Vector2DD normRight = curve->netNormals_[nextIdx].normalized();
            Vector2DD normRight = ((v3 - v2).normalized() + (v1 - v2).normalized()).normalized();
            auto rightNorm = inflNormal(normRight, v1v2, orthogonalV1V2);
            Vector2DD inflNormalRight = rightNorm.first;
            long double rightAngle = rightNorm.second;

            // The location is the midpoint of the edge
            Vector2DD midPoint = mix(v1, v2, ratio);
            // The normal is the normal of either of the edge point normals resulting in the least curvature change (i.e. the flattest curve)
            Vector2DD inflNormal = leftAngle < rightAngle ? inflNormalLeft : inflNormalRight;

            // save
            coords.emplace_back(midPoint);
            norms.emplace_back(inflNormal);
            customNorms.emplace_back(true);
            curve->inflPointsIndices_.insert(idx);
            idx++;
        }
    }
}
std::pair<Vector2DD, long double> ConicSubdivider::inflNormal(const Vector2DD &pointNormal,
                                                              const Vector2DD &edge,
                                                              const Vector2DD &orthogonal) const {
    long double smallestOrthoAngle = std::min(std::abs(std::acos(pointNormal.dot(orthogonal))),
                                              std::abs(std::acos(pointNormal.dot(-orthogonal))));
    long double smallestEdgeAngle = std::min(std::abs(std::acos(pointNormal.dot(edge))),
                                             std::abs(std::acos(pointNormal.dot(-edge))));
    Vector2DD normal;
    long double angle;
    if (smallestEdgeAngle > smallestOrthoAngle) {
        // The normal makes a sharper angle with the orthogonal vector
        // Take the normal and reflect it around the orthogonal vector to obtain the left inflection normal
        normal = pointNormal - 2 * (pointNormal.dot(orthogonal)) * orthogonal;
        normal *= -1;
        normal.normalize();
        angle = smallestOrthoAngle;
    } else {
        // The normal makes a sharper angle with the edge itself. Simply rotate by 90 degrees
        normal = {-pointNormal.y(), pointNormal.x()};
        angle = smallestEdgeAngle;
    }
    return {normal, angle};
}
