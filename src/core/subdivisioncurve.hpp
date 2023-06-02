#pragma once

#include <QString>
#include <QVector>
#include <QSet>
#include "util/vector.hpp"

class Settings;


/**
 * @brief The SubdivisionCurve class contains the data of a 2D subdivision
 * curve.
 */
class SubdivisionCurve {
public:

    explicit SubdivisionCurve(const Settings &settings);

    explicit SubdivisionCurve(const Settings &settings, QVector<Vector2DD> coords, bool closed = true);

    SubdivisionCurve(const Settings &settings, QVector<Vector2DD> coords, QVector<Vector2DD> normals,
                     bool closed = true);

    inline QVector<Vector2DD> getNetCoords() { return netCoords_; }

    inline QVector<Vector2DD> getNetNormals() { return netNormals_; }

    inline QVector<Vector2DD> getCurveCoords() { return curveCoords_; }

    inline QVector<Vector2DD> getCurveNormals() { return curveNormals_; }

    inline int getSubdivLevel() const { return subdivisionLevel_; }

    int findClosestVertex(const Vector2DD &p, double maxDist);

    int findClosestNormal(const Vector2DD &p, double maxDist);

    int addPoint(const Vector2DD &p);

    void setVertexPosition(int idx, const Vector2DD &p);

    void setNormalPosition(int idx, const Vector2DD &p);

    void removePoint(int idx);

    void subdivide(int level);

    void reSubdivide();

    void recalculateNormals();

    void recalculateNormal(int idx);

    bool isClosed() const;

    void setClosed(bool closed);

    void insertKnots();

    void applySubdivision();

    QVector<double> getStabilityVals() const;

    void translate(const Vector2DD &translation);

private:
    const Settings &settings_;

    int subdivisionLevel_ = 0;
    bool closed_ = false;

    QVector<Vector2DD> curveCoords_;
    QVector<Vector2DD> curveNormals_;
    QVector<double> stability;
    QVector<bool> customNormals_;
    QSet<int> knotIndices;

    QVector<Vector2DD> netCoords_;
    QVector<Vector2DD> netNormals_;

    void subdivide(const QVector<Vector2DD> &points,
                   const QVector<Vector2DD> &normals,
                   const QVector<double> &stabilities, int level);

    QVector<Vector2DD> calcNormals(const QVector<Vector2DD> &coords) const;

    void calcNormalAtIndex(const QVector<Vector2DD> &coords, QVector<Vector2DD> &normals, int i) const;

    int findInsertIdx(const Vector2DD &p);

    int getNextIdx(int idx);

    int getPrevIdx(int idx);

    void knotCurve(QVector<Vector2DD> &coords, QVector<Vector2DD> &norms, QVector<bool> &customNorms);

    void extractPatch(const QVector<Vector2DD> &points, const QVector<Vector2DD> &normals, QVector<int> &indices, int i,
                      QVector<Vector2DD> &patchCoords, QVector<Vector2DD> &patchNormals) const;

    bool areInSameHalfPlane(const Vector2DD &v0, const Vector2DD &v1, const Vector2DD &v2, const Vector2DD &v3) const;
};
