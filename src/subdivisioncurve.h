#ifndef SUBDIVISIONCURVE_H
#define SUBDIVISIONCURVE_H

#include <QString>
#include <QVector2D>
#include <QVector>

#include "settings.h"

/**
 * @brief The SubdivisionCurve class contains the data of a 2D subdivision
 * curve.
 */
class SubdivisionCurve {
 public:
  SubdivisionCurve();

  void addSettings(Settings* settings);

  inline QVector<QVector2D> getNetCoords() { return netCoords; }
  inline QVector<QVector2D> getNetNormals() { return netNormals; }
  inline QVector<QVector2D> getCurveCoords() { return curveCoords; }
  inline QVector<QVector2D> getCurveNormals() { return curveNormals; }
  inline int getSubdivLevel() { return subdivisionLevel; }
  void presetNet(int preset);

  int findClosest(const QVector2D& p, const float maxDist);

  void addPoint(QVector2D p);
  void setPointPosition(int idx, QVector2D p);
  void removePoint(int idx);

  void subdivide(int level);
  void reSubdivide();

 private:
  int subdivisionLevel;

  QVector<QVector2D> curveCoords;
  QVector<QVector2D> curveNormals;

  QVector<QVector2D> netCoords;
  QVector<QVector2D> netNormals;
  Settings* settings;

  void subdivide(const QVector<QVector2D>& points,
                 const QVector<QVector2D>& normals, int level);
  QVector<QVector2D> calcNormals(const QVector<QVector2D>& coords) const;
};

#endif  // SUBDIVISIONCURVE_H
