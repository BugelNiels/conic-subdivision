#include "conicsubdivider.hpp"

#include <cmath>

#include "conis/core/conics/conic.hpp"

namespace conis::core {

ConicSubdivider::ConicSubdivider(const SubdivisionSettings &settings) : settings_(settings) {}

void ConicSubdivider::subdivide(Curve &curve, int level) {
    auto &coords = curve.getCoords();
    auto &normals = curve.getNormals();
    int n = coords.size();
    if (n == 0 || level == 0) {
        return;
    }
    if (settings_.convexitySplit) {
        getInflPointCurve(curve, pointsBuffer, normalsBuffer);
        n = pointsBuffer.size();
        // In this case the curve to subdivide is in the buffer, not in the curve
        if (level % 2 == 1) {
            subdivideRecursive(pointsBuffer, normalsBuffer, coords, normals, n, level, curve.isClosed());
        } else {
            coords.resize(n);
            normals.resize(n);
            // Ensure the final result always ends up in the curve itself again
            std::copy(pointsBuffer.begin(), pointsBuffer.end(), coords.begin());
            std::copy(normalsBuffer.begin(), normalsBuffer.end(), normals.begin());
            subdivideRecursive(coords, normals, pointsBuffer, normalsBuffer, n, level, curve.isClosed());
        }
    } else {
        inflPointIndices_.clear();
        if (level % 2 == 1) {
            pointsBuffer.resize(n);
            normalsBuffer.resize(n);
            // Ensure the final result always ends up in the curve itself again
            std::copy(coords.begin(), coords.begin() + n, pointsBuffer.begin());
            std::copy(normals.begin(), normals.begin() + n, normalsBuffer.begin());
            subdivideRecursive(pointsBuffer, normalsBuffer, coords, normals, n, level, curve.isClosed());
        } else {
            subdivideRecursive(coords, normals, pointsBuffer, normalsBuffer, n, level, curve.isClosed());
        }
    }
}

void ConicSubdivider::subdivideRecursive(std::vector<Vector2DD> &points,
                                         std::vector<Vector2DD> &normals,
                                         std::vector<Vector2DD> &newPoints,
                                         std::vector<Vector2DD> &newNormals,
                                         int numPoints,
                                         int level,
                                         bool closed) {
    // base case
    if (level == 0) {
        return;
    }
    int n = numPoints * 2 - 1;
    if (closed) {
        n += 1;
    }
    newPoints.resize(n);
    newNormals.resize(n);

    // set old vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
    }
    // set new edge points
    for (int i = 1; i < n; i += 2) {
        edgePoint(points, normals, newPoints, newNormals, i, closed);
    }
    // Update the indices of the inflection points
    for (int &inflIdx: inflPointIndices_) {
        inflIdx *= 2;
    }
    subdivideRecursive(newPoints, newNormals, points, normals, n, level - 1, closed);
}

void ConicSubdivider::edgePoint(const std::vector<Vector2DD> &points,
                                const std::vector<Vector2DD> &normals,
                                std::vector<Vector2DD> &newPoints,
                                std::vector<Vector2DD> &newNormals,
                                int i,
                                bool closed) const {
    const int n = newPoints.size();
    std::vector<PatchPoint> patchPoints = extractPatch(points, normals, i / 2, settings_.patchSize, closed);

    const int prevIdx = (i - 1 + n) % n;
    const int nextIdx = (i + 1) % n;

    const Vector2DD origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0;
    Vector2DD dir = newPoints[prevIdx] - newPoints[nextIdx];
    dir = {-dir.y(), dir.x()};
    // Note that dir is not normalized!
    Conic conic(patchPoints, settings_.epsilon);
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
                patchPoints = extractPatch(points, normals, i / 2, patchSize, closed);
                if (patchSize > 10 || patchPoints.size() == oldPatchSize) {
                    sampledPoint = origin;
                    sampledNormal = dir;
                    break;
                }
                oldPatchSize = patchPoints.size();
                Conic conic2(patchPoints, settings_.epsilon);
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
                                                      int maxPatchSize,
                                                      bool closed) const {

    std::vector<PatchPoint> patchPoints;
    patchPoints.reserve(4);
    const int size = int(points.size());
    // Left middle
    const int leftMiddleIdx = pIdx;
    const bool leftInflPoint = std::find(inflPointIndices_.begin(), inflPointIndices_.end(), leftMiddleIdx) !=
                               inflPointIndices_.end();
    patchPoints.push_back(
            {points[leftMiddleIdx], normals[leftMiddleIdx], settings_.middlePointWeight, settings_.middleNormalWeight});

    // Right middle
    const int rightMiddleIdx = (pIdx + 1) % size;
    const bool rightInflPoint = std::find(inflPointIndices_.begin(), inflPointIndices_.end(), rightMiddleIdx) !=
                                inflPointIndices_.end();
    patchPoints.push_back({points[rightMiddleIdx],
                           normals[rightMiddleIdx],
                           settings_.middlePointWeight,
                           settings_.middleNormalWeight});

    if (closed) {
        if (!leftInflPoint) {
            const int leftOuterIdx = (leftMiddleIdx - 1 + size) % size;
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = (leftMiddleIdx - i + size) % size;
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[leftOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back(
                        {points[idx], normals[idx], settings_.outerPointWeight, settings_.outerNormalWeight});
            }
        }
        if (!rightInflPoint) {
            int rightOuterIdx = (rightMiddleIdx + 1) % size;
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = (rightMiddleIdx + i) % size;
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[rightOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back(
                        {points[idx], normals[idx], settings_.outerPointWeight, settings_.outerNormalWeight});
            }
        }
    } else {
        if (!leftInflPoint) {
            const int leftOuterIdx = leftMiddleIdx - 1;
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = leftMiddleIdx - i;
                if (idx < 0 || leftOuterIdx < 0) {
                    break;
                }
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[leftOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back(
                        {points[idx], normals[idx], settings_.outerPointWeight, settings_.outerNormalWeight});
            }
        }
        if (!rightInflPoint) {
            const int rightOuterIdx = rightMiddleIdx + 1;
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = rightMiddleIdx + i;
                if (idx >= size || rightOuterIdx >= size) {
                    break;
                }
                if (!areInSameHalfPlane(points[leftMiddleIdx],
                                        points[rightMiddleIdx],
                                        points[rightOuterIdx],
                                        points[idx])) {
                    break;
                }
                patchPoints.push_back(
                        {points[idx], normals[idx], settings_.outerPointWeight, settings_.outerNormalWeight});
            }
        }
    }
    return patchPoints;
}

bool ConicSubdivider::areInSameHalfPlane(const Vector2DD &v0,
                                         const Vector2DD &v1,
                                         const Vector2DD &v2,
                                         const Vector2DD &v3) const {
    const Vector2DD v1v3 = v3 - v1;
    const Vector2DD v1v0 = v0 - v1;
    if (v1v0.squaredNorm() == 0.0 || v1v3.squaredNorm() == 0) {
        return true; // End point edge case
    }
    const Vector2DD normal = Vector2DD(v2.y() - v1.y(), v1.x() - v2.x());
    const real_t dotProduct1 = normal.dot(v1v3);
    const real_t dotProduct2 = normal.dot(v1v0);
    if (std::abs(dotProduct1) < settings_.epsilon || std::abs(dotProduct2) < settings_.epsilon) {
        return true; // curve is flat
    }
    const double sign = dotProduct1 > 0 ? 1.0 : -1.0;
    return dotProduct2 * sign >= 0;
}

std::vector<bool> ConicSubdivider::getInflPointCurve(const Curve &curve,
                                                     std::vector<Vector2DD> &coords,
                                                     std::vector<Vector2DD> &normals) {
    // Setup
    const int n = int(curve.numPoints());
    coords.resize(0);
    normals.resize(0);
    inflPointIndices_.clear();
    inflPointIndices_.reserve(n);
    std::vector<bool> customNorms;
    customNorms.reserve(n);
    int idx = 0;
    // For all points
    for (int i = 0; i < n; i++) {
        int nextIdx = curve.getNextIdx(i);
        const Vector2DD &v0 = curve.getCoords()[curve.getPrevIdx(i)];
        const Vector2DD &v1 = curve.getCoords()[i];
        const Vector2DD &v2 = curve.getCoords()[nextIdx];
        const Vector2DD &v3 = curve.getCoords()[curve.getNextIdx(nextIdx)];

        // Insert original point and normal
        coords.emplace_back(v1);
        normals.emplace_back(curve.getNormals()[i]);
        customNorms.emplace_back(curve.getCustomNormals()[i]);
        idx++;
        // For non-closed curves or when there are not enough points (<= 2)
        if (v0 == v1 || v2 == v3 || v1 == v2) {
            continue;
        }

        // Insert an inflection point
        if (!areInSameHalfPlane(v0, v1, v2, v3)) {
            const Vector2DD v0v1 = (v0 - v1).normalized();
            const Vector2DD v1v2 = (v1 - v2).normalized();
            const Vector2DD v3v2 = (v3 - v2).normalized();
            const Vector2DD orthogonalV1V2 = {-v1v2.y(), v1v2.x()};

            real_t ratio = 0.5;
            if (settings_.weightedInflPointLocation) {
                const real_t dot1 = (v0 - v1).normalized().dot((v1 - v2).normalized());
                const real_t dot2 = (v3 - v2).normalized().dot((v2 - v1).normalized());
                const real_t l1 = std::abs(std::acos(dot1));
                const real_t l2 = std::abs(std::acos(dot2));
                if (settings_.gravitateSmallerAngles) {
                    ratio = l1 / (l1 + l2);
                } else {
                    ratio = l2 / (l1 + l2);
                }
            }

            auto [inflNormalLeft, leftAngle] = inflNormal(v0v1, -v1v2, orthogonalV1V2);
            auto [inflNormalRight, rightAngle] = inflNormal(v3v2, v1v2, orthogonalV1V2);
            // The location is the midpoint of the edge
            const Vector2DD midPoint = mix(v1, v2, ratio);
            // The normal is the normal of either of the edge point normals resulting in the least curvature change (i.e. the flattest curve)
            const Vector2DD inflNormal = leftAngle < rightAngle ? inflNormalLeft : inflNormalRight;
            // save
            coords.emplace_back(midPoint);
            normals.emplace_back(inflNormal);
            customNorms.emplace_back(true);
            inflPointIndices_.push_back(idx);
            idx++;
        }
    }
    return customNorms;
}

std::pair<Vector2DD, real_t> ConicSubdivider::inflNormal(const Vector2DD &edgeAB,
                                                         const Vector2DD &edgeBC,
                                                         const Vector2DD &orthogonal) const {
    // angle is between is between pi and 0
    const real_t angle = std::acos(edgeAB.normalized().dot(edgeBC.normalized()));
    //               angle  / M_PI        is between 1 and 0
    //               angle  / M_PI - 0.5  is between 0.5 and -0.5
    //      std::abs(angle) / M_PI - 0.5) is between 0.5 and 0
    // 0.5  std::abs(angle) / M_PI - 0.5) is between 0 and 0.5
    const real_t gamma = 0.5 - std::abs(angle / M_PI - 0.5);
    // Set the normal in the correct direction to ensure the inflection normal makes the correct angle
    const auto reflectFlatNormal = edgeAB.dot(orthogonal) < 0 ? edgeBC : -edgeBC;
    // linear blend, gamma is in the range [0,0.5]
    Vector2DD normal = mix(orthogonal, reflectFlatNormal, gamma).normalized();

    // Make sure we use the orthogonal vector pointing in the same general direction as the normal
    const Vector2DD correctedOrtho = normal.dot(orthogonal) < 0 ? -orthogonal : orthogonal;
    if (settings_.areaWeightedNormals) {
        // Mix between the orthogonal vector and the found normal depending on the length ratio between the edges.
        // This ensures a flatter curve when one edge is disproportionally large compared to the other
        const real_t lr = std::abs(edgeAB.norm() / (edgeAB.norm() + edgeBC.norm()) - 0.5) * 2;
        normal = mix(normal, correctedOrtho, lr);
    }
    // The angle the normal makes with the orthogonal vector
    const real_t angleOrtho = std::abs(std::acos(normal.dot(correctedOrtho)));
    return {normal, angleOrtho};
}

} // namespace conis::core