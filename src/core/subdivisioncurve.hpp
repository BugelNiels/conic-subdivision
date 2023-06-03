#pragma once

#include <QString>
#include <QVector>
#include <QSet>
#include "util/vector.hpp"
#include "core/subdivision/conicsubdivider.hpp"

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

    inline const QVector<Vector2DD>& getNetCoords() const { return netCoords_; }

    inline const QVector<Vector2DD>& getNetNormals() const { return netNormals_; }

    inline const QVector<Vector2DD>& getCurveCoords() const { return curveCoords_; }

    inline const QVector<Vector2DD>& getCurveNormals() const { return curveNormals_; }

    inline int getSubdivLevel() const { return subdivisionLevel_; }

    int findClosestVertex(const Vector2DD &p, double maxDist) const;

    int findClosestNormal(const Vector2DD &p, double maxDist) const;

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
    ConicSubdivider subdivider;

    int subdivisionLevel_ = 0;
    bool closed_ = false;

    QVector<Vector2DD> curveCoords_;
    QVector<Vector2DD> curveNormals_;
    QVector<bool> customNormals_;
    QSet<int> knotIndices_;

    QVector<Vector2DD> netCoords_;
    QVector<Vector2DD> netNormals_;


    QVector<Vector2DD> calcNormals(const QVector<Vector2DD> &coords) const;

    Vector2DD calcNormalAtIndex(const QVector<Vector2DD> &coords, const QVector<Vector2DD> &normals, int i) const;

    int findInsertIdx(const Vector2DD &p) const;

    int getNextIdx(int idx) const;

    int getPrevIdx(int idx) const;

    friend class ConicSubdivider;

};
