#pragma once


#include "core/settings.hpp"
#include "util/vector.hpp"

class SubdivisionCurve;

class ConicSubdivider {
public:
    explicit ConicSubdivider(const Settings &settings);

    void subdivide(SubdivisionCurve *curve, int level);

    void knotCurve(SubdivisionCurve *curve, QVector<Vector2DD> &coords, QVector<Vector2DD> &norms,
                   QVector<bool> &customNorms);

    QVector<double> getStabilityVals() const;

private:
    const Settings &settings_;
    QVector<double> stability_;
    SubdivisionCurve *curve_;

    void subdivide(const QVector<Vector2DD> &points,
                   const QVector<Vector2DD> &normals,
                   const QVector<double> &stabilities, int level);

    bool areInSameHalfPlane(const Vector2DD &v0, const Vector2DD &v1, const Vector2DD &v2, const Vector2DD &v3) const;

    void extractPatch(const QVector<Vector2DD> &points, const QVector<Vector2DD> &normals,
                      QVector<int> &indices, int i, QVector<Vector2DD> &patchCoords,
                      QVector<Vector2DD> &patchNormals) const;
};
