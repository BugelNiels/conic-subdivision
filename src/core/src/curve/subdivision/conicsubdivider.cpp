#include "conis/core/curve/subdivision/conicsubdivider.hpp"

#include <cmath>
#include <iostream>

#include "conis/core/conics/conic.hpp"

namespace conis::core {

ConicSubdivider::ConicSubdivider(const SubdivisionSettings &settings)
    : settings_(settings),
      fitter_(settings.epsilon) {}

void ConicSubdivider::subdivide(Curve &curve, const int level) {
    if (curve.numPoints() == 0 || level == 0) {
        return;
    }
    bufferCurve_.setClosed(curve.isClosed(), false);
    if (settings_.convexitySplit) {
        // Insert directly into the buffer curve to prevent re-allocations
        insertInflPoints(curve, bufferCurve_);
        // In this case the curve to subdivide is in the buffer, not in the curve
        if (level % 2 == 1) {
            subdivideRecursive(bufferCurve_, curve, level);
        } else {
            // Ensure the final result always ends up in the curve itself again
            bufferCurve_.copyDataTo(curve);
            subdivideRecursive(curve, bufferCurve_, level);
        }
    } else {
        inflPointIndices_.clear();
        if (level % 2 == 1) {
            // Ensure the final result always ends up in the curve itself again
            curve.copyDataTo(bufferCurve_);
            subdivideRecursive(bufferCurve_, curve, level);
        } else {
            subdivideRecursive(curve, bufferCurve_, level);
        }
    }
    curve.getCustomNormals().resize(curve.numPoints());
}

void ConicSubdivider::subdivideRecursive(Curve &controlCurve, Curve &subdivCurve, const int level) {
    // base case
    if (level == 0) {
        return;
    }
    int n = controlCurve.numPoints() * 2 - 1;
    if (controlCurve.isClosed()) {
        n += 1;
    }
    subdivCurve.getVertices().resize(n);
    subdivCurve.getNormals().resize(n);

    // set old vertex points
    for (int i = 0; i < n; i += 2) {
        subdivCurve.setVertex(i, controlCurve.getVertex(i / 2));
        subdivCurve.setNormal(i, controlCurve.getNormal(i / 2));
    }
    // set new edge points
    for (int i = 1; i < n; i += 2) {
        edgePoint(controlCurve, subdivCurve, i);
    }
    // Update the indices of the inflection points
    for (int &inflIdx: inflPointIndices_) {
        inflIdx *= 2;
    }
    // Double buffering: switch the curves
    subdivideRecursive(subdivCurve, controlCurve, level - 1);
}

void ConicSubdivider::edgePoint(const Curve &controlCurve, Curve &subdivCurve, const int i) {
    const int n = subdivCurve.numPoints();
    const int prevIdx = (i - 1 + n) % n;
    const int nextIdx = (i + 1) % n;

    // Construct origin and direction of the ray we use to intersect the found conic
    const Vector2DD origin = (subdivCurve.getVertex(prevIdx) + subdivCurve.getVertex(nextIdx)) / 2.0;
    Vector2DD dir = subdivCurve.getVertex(prevIdx) - subdivCurve.getVertex(nextIdx);
    // rotate the line segment counterclockwise 90 degree to get the normal of it
    // Note that dir is not normalized! This is to prevent introducing further rounding errors.
    // The conic solver finds a multiple of the normal, so the length does not matter
    dir = {dir.y(), -dir.x()};

    // i/2 as we extract the patch from the control curve (while we're currently in the index space of the subdiv curve)
    std::vector<PatchPoint> patchPoints = extractPatch(controlCurve, i / 2, settings_.patchSize);
    const Conic conic = fitter_.fitConic(patchPoints);
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
                patchPoints = extractPatch(controlCurve, i / 2, patchSize);
                // 4 is the absolute max patch size (number of points on either side, so 9 in total max)
                if (patchSize > 4 || patchPoints.size() == oldPatchSize) {
                    sampledPoint = origin;
                    sampledNormal = dir;
                    break;
                }
                oldPatchSize = patchPoints.size();
                const Conic conic2 = fitter_.fitConic(patchPoints);
                valid = conic2.sample(origin, dir, sampledPoint, sampledNormal);
                patchSize++;
            }
        } else {
            // No valid conic found, set to midpoint and its normal
            sampledPoint = origin;
            sampledNormal = dir;
        }
    }
#ifdef NORMALIZE_CONIC_NORMALS
    sampledNormal.normalize();
#endif
    subdivCurve.setVertex(i, sampledPoint);
    subdivCurve.setNormal(i, sampledNormal);
}

std::vector<PatchPoint> ConicSubdivider::extractPatch(const Curve &curve,
                                                      const int pIdx,
                                                      const int maxPatchSize) const {
    const auto &verts = curve.getVertices();
    const auto &normals = curve.getNormals();
    std::vector<PatchPoint> patchPoints;
    patchPoints.reserve(4);
    const int n = curve.numPoints();
    // Left middle
    const int leftMiddleIdx = pIdx;
    const bool leftInflPoint = std::find(inflPointIndices_.begin(), inflPointIndices_.end(), leftMiddleIdx) !=
                               inflPointIndices_.end();
    patchPoints.emplace_back(verts[leftMiddleIdx],
                             normals[leftMiddleIdx],
                             settings_.middlePointWeight,
                             settings_.middleNormalWeight);

    // Right middle
    const int rightMiddleIdx = curve.getNextIdx(pIdx);
    const bool rightInflPoint = std::find(inflPointIndices_.begin(), inflPointIndices_.end(), rightMiddleIdx) !=
                                inflPointIndices_.end();
    patchPoints.emplace_back(verts[rightMiddleIdx],
                             normals[rightMiddleIdx],
                             settings_.middlePointWeight,
                             settings_.middleNormalWeight);

    if (curve.isClosed()) {
        if (!leftInflPoint) {
            const int leftOuterIdx = curve.getPrevIdx(leftMiddleIdx);
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = (leftMiddleIdx - i + n) % n;
                if (!areInSameHalfPlane(verts[leftMiddleIdx], verts[rightMiddleIdx], verts[leftOuterIdx], verts[idx])) {
                    break;
                }
                patchPoints.emplace_back(verts[idx],
                                         normals[idx],
                                         settings_.outerPointWeight,
                                         settings_.outerNormalWeight);
            }
        }
        if (!rightInflPoint) {
            int rightOuterIdx = curve.getNextIdx(rightMiddleIdx);
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = (rightMiddleIdx + i) % n;
                if (!areInSameHalfPlane(verts[leftMiddleIdx],
                                        verts[rightMiddleIdx],
                                        verts[rightOuterIdx],
                                        verts[idx])) {
                    break;
                }
                patchPoints.emplace_back(verts[idx],
                                         normals[idx],
                                         settings_.outerPointWeight,
                                         settings_.outerNormalWeight);
            }
        }
    } else {
        if (!leftInflPoint) {
            const int leftOuterIdx = curve.getPrevIdx(leftMiddleIdx);
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = leftMiddleIdx - i;
                if (idx < 0 || leftOuterIdx < 0) {
                    break;
                }
                if (!areInSameHalfPlane(verts[leftMiddleIdx], verts[rightMiddleIdx], verts[leftOuterIdx], verts[idx])) {
                    break;
                }
                patchPoints.emplace_back(verts[idx],
                                         normals[idx],
                                         settings_.outerPointWeight,
                                         settings_.outerNormalWeight);
            }
        }
        if (!rightInflPoint) {
            const int rightOuterIdx = curve.getNextIdx(rightMiddleIdx);
            for (int i = 1; i < maxPatchSize; ++i) {
                const int idx = rightMiddleIdx + i;
                if (idx >= n || rightOuterIdx >= n) {
                    break;
                }
                if (!areInSameHalfPlane(verts[leftMiddleIdx],
                                        verts[rightMiddleIdx],
                                        verts[rightOuterIdx],
                                        verts[idx])) {
                    break;
                }
                patchPoints.emplace_back(verts[idx],
                                         normals[idx],
                                         settings_.outerPointWeight,
                                         settings_.outerNormalWeight);
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
    if (v1v0.squaredNorm() < settings_.epsilon || v1v3.squaredNorm() < settings_.epsilon) {
        return true; // End point edge case
    }
    const Vector2DD normal = Vector2DD(v2.y() - v1.y(), v1.x() - v2.x());
    const real_t dotProduct1 = normal.dot(v1v3);
    const real_t dotProduct2 = normal.dot(v1v0);
    if (std::abs(dotProduct1) < settings_.epsilon || std::abs(dotProduct2) < settings_.epsilon) {
        return true; // curve is flat
    }
    const real_t sign = dotProduct1 > 0 ? 1.0 : -1.0;
    return dotProduct2 * sign >= 0;
}

void ConicSubdivider::insertInflPoints(const Curve &curve, Curve &targetCurve) {
    targetCurve.setClosed(curve.isClosed(), false);
    // Setup
    const int n = int(curve.numPoints());
    inflPointIndices_.clear();
    inflPointIndices_.reserve(n);
    auto &verts = targetCurve.getVertices();
    auto &normals = targetCurve.getNormals();
    auto &customNormals = targetCurve.getCustomNormals();
    verts.clear();
    normals.clear();
    customNormals.clear();

    int idx = 0;
    // For all points
    for (int i = 0; i < n; i++) {
        const int nextIdx = curve.getNextIdx(i);
        const Vector2DD &v0 = curve.getVertex(curve.getPrevIdx(i));
        const Vector2DD &v1 = curve.getVertex(i);
        const Vector2DD &v2 = curve.getVertex(nextIdx);
        const Vector2DD &v3 = curve.getVertex(curve.getNextIdx(nextIdx));

        // Insert original point and normal
        verts.emplace_back(v1);
        normals.emplace_back(curve.getNormal(i));
        customNormals.emplace_back(curve.isCustomNormal(i));
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
            verts.emplace_back(midPoint);
            normals.emplace_back(inflNormal);
            customNormals.emplace_back(true);
            inflPointIndices_.push_back(idx);
            idx++;
            // std::cout << "Inserting inflection point" << std::endl;
        }
    }
}

Curve ConicSubdivider::getInflPointCurve(const Curve &curve) {
    Curve inflCurve;
    insertInflPoints(curve, inflCurve);
    return inflCurve;
}

std::pair<Vector2DD, real_t> ConicSubdivider::inflNormal(const Vector2DD &edgeAB,
                                                         const Vector2DD &edgeBC,
                                                         const Vector2DD &orthogonal) const {
    // angle is between pi and 0
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