#pragma once

#include <QString>
#include <QVector2D>
#include <QVector>

class Settings;

/**
 * @brief The SubdivisionCurve class contains the data of a 2D subdivision
 * curve.
 */
class SubdivisionCurve {
public:

    explicit SubdivisionCurve(const Settings &settings);

    explicit SubdivisionCurve(const Settings &settings, QVector<QVector2D> coords, bool closed = true);

    SubdivisionCurve(const Settings &settings, QVector<QVector2D> coords, QVector<QVector2D> normals, bool closed = true);

    inline QVector<QVector2D> getNetCoords() { return netCoords_; }

    inline QVector<QVector2D> getNetNormals() { return netNormals_; }

    inline QVector<QVector2D> getCurveCoords() { return curveCoords_; }

    inline QVector<QVector2D> getCurveNormals() { return curveNormals_; }

    inline int getSubdivLevel() const { return subdivisionLevel_; }

    int findClosestVertex(const QVector2D &p, float maxDist);

    int findClosestNormal(const QVector2D &p, float maxDist);

    int addPoint(QVector2D p);

    void flipNormals();

    void setVertexPosition(int idx, QVector2D p);

    void setNormalPosition(int idx, QVector2D p);

    void removePoint(int idx);

    void subdivide(int level);

    void reSubdivide();

    void recalculateNormals();

    void recalculateNormal(int idx);

    bool isClosed() const;

    void setClosed(bool closed);

    void insertKnots();

    void applySubdivision();

    QVector<float> getStabilityVals() const;

    void translate(const QVector2D& translation);

private:
    const Settings &settings_;

    int subdivisionLevel_ = 0;
    bool closed_ = false;

    QVector<QVector2D> curveCoords_;
    QVector<QVector2D> curveNormals_;
    QVector<float> stability;
    QVector<bool> customNormals_;

    QVector<QVector2D> netCoords_;
    QVector<QVector2D> netNormals_;

    void subdivide(const QVector<QVector2D> &points,
                   const QVector<QVector2D> &normals,
                   const QVector<float> &stabilities, int level);

    QVector<QVector2D> calcNormals(const QVector<QVector2D> &coords) const;

    void calcNormalAtIndex(const QVector<QVector2D> &coords, QVector<QVector2D> &normals, int i) const;

    int findInsertIdx(const QVector2D &p);

    int getNextIdx(int idx);

    int getPrevIdx(int idx);

    void knotSubdivide(int level);

    void knotCurve(QVector<QVector2D> &coords, QVector<QVector2D> &norms, QVector<bool> &customNorms);

    void tessellate(const QVector<QVector2D> &points, const QVector<QVector2D> &normals, int level);
};
