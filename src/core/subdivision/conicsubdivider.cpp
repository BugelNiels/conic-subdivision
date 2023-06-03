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
        std::vector<double> stabilities;
        stabilities.resize(coords.size());
        subdivide(coords, norms, stabilities, level);
    } else {
        stability_.resize(netCoords.size());
        subdivide(netCoords, netNorms, stability_, level);
    }

}

void ConicSubdivider::subdivide(const std::vector<Vector2DD> &points,
                                const std::vector<Vector2DD> &normals,
                                const std::vector<double> &stabilities, int level) {
    // base case
    if (level == 0) {
        curve_->curveCoords_ = points;
        curve_->curveNormals_ = normals;
        stability_ = stabilities;
        return;
    }
    int n = int(points.size()) * 2 - 1;
    if (curve_->isClosed()) {
        n += 1;
    }
    std::vector<Vector2DD> newPoints(n);
    std::vector<Vector2DD> newNormals(n);
    std::vector<double> newStabilities(n);

    // set vertex points
    for (int i = 0; i < n; i += 2) {
        newPoints[i] = points[i / 2];
        newNormals[i] = normals[i / 2];
        newStabilities[i] = stabilities[i / 2];
    }
    // set new points
    std::vector<int> indices;
    indices.reserve(4);
    for (int i = 1; i < n; i += 2) {
        std::vector<Vector2DD> patchCoords;
        std::vector<Vector2DD> patchNormals;

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
        newNormals = curve_->calcNormals(newPoints);
    }
    curve_->knotIndices_.clear();
    subdivide(newPoints, newNormals, newStabilities, level - 1);
}


bool arePointsCollinear(const Vector2DD &p1, const Vector2DD &p2, const Vector2DD &p3) {
    return p1.x() * (p2.y() - p3.y()) + p2.x() * (p3.y() - p1.y()) + p3.x() * (p1.y() - p2.y()) < 0.0001;
}

void ConicSubdivider::extractPatch(const std::vector<Vector2DD> &points, const std::vector<Vector2DD> &normals,
                                   std::vector<int> &indices, int i, std::vector<Vector2DD> &patchCoords,
                                   std::vector<Vector2DD> &patchNormals) const {
    int pIdx = i / 2;

    int size = int(points.size());

    indices.clear();
    // Two edge points
    indices.emplace_back(pIdx);
    // p_2-p_1-p0-p1-p2-p3
    if (curve_->isClosed()) {
        bool startKnotPoint = curve_->knotIndices_.contains(pIdx);
        bool endKnotPoint = curve_->knotIndices_.contains((pIdx + 1) % size);
        indices.emplace_back((pIdx + 1) % size);
        if (!startKnotPoint) {
            indices.emplace_back((pIdx - 1 + size) % size);
        } else {
            indices.emplace_back((pIdx + 2) % size);
        }
        if (!endKnotPoint) {
            indices.emplace_back((pIdx + 2) % size);
        } else {
            indices.emplace_back((pIdx - 1 + size) % size);
        }
    } else {
        indices.emplace_back(pIdx + 1);
        if (pIdx + 2 < size) {
            indices.emplace_back(pIdx + 2);
        }
        if (pIdx - 1 >= 0) {
            indices.emplace_back(pIdx - 1);
        }
    }
    for (int index: indices) {
        patchCoords.emplace_back(points[index]);
        patchNormals.emplace_back(normals[index]);
    }
}


bool ConicSubdivider::areInSameHalfPlane(const Vector2DD &v0, const Vector2DD &v1, const Vector2DD &v2,
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

void ConicSubdivider::knotCurve(SubdivisionCurve *curve, std::vector<Vector2DD> &coords, std::vector<Vector2DD> &norms,
                                std::vector<bool> &customNorms) {


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

            Vector2DD reflectVec = (v1 - v2).normalized();
            reflectVec = {-reflectVec.y(), reflectVec.x()};
            double angle1 = curve->netNormals_[i].dot(reflectVec);
            double angle2 = curve->netNormals_[nextIdx].dot(reflectVec);

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
            Vector2DD incident = mix(curve->netNormals_[i], curve->netNormals_[nextIdx], ratio).normalized();
            incident *= -1;
            incident = angle1 > angle2 ? curve->netNormals_[i] : curve->netNormals_[nextIdx];

            Vector2DD knotNormal = incident - 2 * (incident.dot(reflectVec)) * reflectVec; // reflect
            knotNormal *= -1;
            knotNormal.normalize();

            knotNormal = (1 - settings_.knotTension) * knotNormal + settings_.knotTension * reflectVec;
            knotNormal.normalize();
            coords.emplace_back(midPoint);
            norms.emplace_back(knotNormal);
            customNorms.emplace_back(true);
            curve->knotIndices_.insert(idx);
            idx++;
        }
    }
}

std::vector<double> ConicSubdivider::getStabilityVals() const {
    return stability_;
}
