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
    SubdivisionCurve();

    explicit SubdivisionCurve(Settings *settings, QVector<QVector2D> coords);

    SubdivisionCurve(Settings *settings, QVector<QVector2D> coords, QVector<QVector2D> normals);

    inline QVector<QVector2D> getNetCoords() { return netCoords_; }

    inline QVector<QVector2D> getNetNormals() { return netNormals_; }

    inline QVector<QVector2D> getCurveCoords() { return curveCoords_; }

    inline QVector<QVector2D> getCurveNormals() { return curveNormals_; }

    inline int getSubdivLevel() { return subdivisionLevel_; }

    int findClosestVertex(const QVector2D &p, const float maxDist);

    int findClosestNormal(const QVector2D &p, const float maxDist);

    void addPoint(QVector2D p);

    void flipNormals();

    void setVertexPosition(int idx, QVector2D p);
    void setNormalPosition(int idx, QVector2D p);

    void removePoint(int idx);

    void subdivide(int level);

    void reSubdivide();

    void recalculateNormals();

private:
    int subdivisionLevel_;

    QVector<QVector2D> curveCoords_;
    QVector<QVector2D> curveNormals_;

    QVector<QVector2D> netCoords_;
    QVector<QVector2D> netNormals_;
    Settings *settings_;

    void subdivide(const QVector<QVector2D> &points,
                   const QVector<QVector2D> &normals, int level);

    QVector<QVector2D> calcNormals(const QVector<QVector2D> &coords) const;
    void calcNormalAtIndex(const QVector<QVector2D> &coords, QVector<QVector2D> &normals, int i) const;
};
