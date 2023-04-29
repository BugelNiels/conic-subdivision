#include "subdivisioncurve.h"

#include <QDebug>
#include <QTextStream>

#include "conicfitter.h"

#define MIN(A, B) (A) < (B) ? (A) : (B)
#define MAX(A, B) (A) > (B) ? (A) : (B)

/**
 * @brief SubdivisionCurve::SubdivisionCurve Creates a new subdivision curve.
 */
SubdivisionCurve::SubdivisionCurve() : subdivisionLevel(0) {}

void SubdivisionCurve::addSettings(Settings* settings) {
  this->settings = settings;
}

QVector2D calcNormal(const QVector2D& a, const QVector2D& b,
                     const QVector2D& c) {
  if (a == b) {
    QVector2D normal = c - b;
    normal.setX(normal.x() * -1);
    return QVector2D(normal.y(), normal.x()).normalized();
  }
  if (b == c) {
    QVector2D normal = b - a;
    normal.setX(normal.x() * -1);
    return QVector2D(normal.y(), normal.x()).normalized();
  }
  QVector2D t1 = (a - b);
  t1 = {-t1.y(), t1.x()};
  QVector2D t2 = (b - c);
  t2 = {-t2.y(), t2.x()};
  if (true) {
    t1.normalize();
    t2.normalize();
  }

  return (t1 + t2).normalized();
  //  return (((a - b).normalized() + (c - b).normalized()) / 2.0).normalized()
  //  *
  //         -1;
  //  return (b - (b + (c - b).normalized() + (a -
  //  b).normalized())).normalized();
}

QVector<QVector2D> SubdivisionCurve::calcNormals(
    const QVector<QVector2D>& coords) const {
  QVector<QVector2D> normals;
  int n = coords.size();
  normals.resize(n);
  for (int i = 0; i < n; i++) {
    int nextIdx, prevIdx;
    if (settings->closed) {
      prevIdx = (i - 1 + n) % n;
      nextIdx = (i + 1) % n;
    } else {
      prevIdx = MAX(0, i - 1);
      nextIdx = MIN(i + 1, n - 1);
    }
    QVector2D a = coords[prevIdx];
    QVector2D b = coords[i];
    QVector2D c = coords[nextIdx];
    if (settings->circleNormals) {
      if (a == b) {
        QVector2D normal = c - b;
        normal.setX(normal.x() * -1);
        normals[i] = QVector2D(normal.y(), normal.x()).normalized();
      } else if (b == c) {
        QVector2D normal = b - a;
        normal.setX(normal.x() * -1);
        normals[i] = QVector2D(normal.y(), normal.x()).normalized();
      } else {
        float d = 2 * (a.x() * (b.y() - c.y()) + b.x() * (c.y() - a.y()) +
                       c.x() * (a.y() - b.y()));
        float ux = ((a.x() * a.x() + a.y() * a.y()) * (b.y() - c.y()) +
                    (b.x() * b.x() + b.y() * b.y()) * (c.y() - a.y()) +
                    (c.x() * c.x() + c.y() * c.y()) * (a.y() - b.y())) /
                   d;
        float uy = ((a.x() * a.x() + a.y() * a.y()) * (c.x() - b.x()) +
                    (b.x() * b.x() + b.y() * b.y()) * (a.x() - c.x()) +
                    (c.x() * c.x() + c.y() * c.y()) * (b.x() - a.x())) /
                   d;
        QVector2D oscCircleCenter = QVector2D(ux, uy);
        normals[i] = (oscCircleCenter - b).normalized();

        QVector2D check = calcNormal(a, b, c);
        if (QVector2D::dotProduct(check, normals[i]) < 0) {
          normals[i] *= -1;
        }
      }
    } else {
      normals[i] = calcNormal(a, b, c);
    }
    // ensure pointing in the same direction
    if (settings->outwardNormals) {
      normals[i] *= -1;
    }
  }

  return normals;
}

/**
 * @brief SubdivisionCurve::presetNet Constructs a control net preset based on a
 * dropdown index.
 * @param preset Index of the preset.
 */
void SubdivisionCurve::presetNet(int preset) {
  netCoords.clear();
  netNormals.clear();
  curveCoords.clear();
  curveNormals.clear();
  subdivisionLevel = 0;

  switch (preset) {
    case 0:
      // Pentagon
      netCoords.reserve(5);
      netCoords.append(QVector2D(-0.25f, -0.5f));
      netCoords.append(QVector2D(-0.75f, 0.0f));
      netCoords.append(QVector2D(-0.25f, 0.75f));
      netCoords.append(QVector2D(0.75f, 0.5f));
      netCoords.append(QVector2D(0.5f, -0.75f));
      break;
    case 1:
      // Basis
      netCoords.reserve(9);
      netCoords.append(QVector2D(-1.0f, -0.25f));
      netCoords.append(QVector2D(-0.75f, -0.25f));
      netCoords.append(QVector2D(-0.5f, -0.25f));
      netCoords.append(QVector2D(-0.25f, -0.25f));
      netCoords.append(QVector2D(0.0f, 0.50f));
      netCoords.append(QVector2D(0.25f, -0.25f));
      netCoords.append(QVector2D(0.5f, -0.25f));
      netCoords.append(QVector2D(0.75f, -0.25f));
      netCoords.append(QVector2D(1.0f, -0.25f));
      break;
    case 2:
      // G
      netCoords.reserve(14);
      netCoords.append(QVector2D(0.75f, 0.35f));
      netCoords.append(QVector2D(0.75f, 0.75f));
      netCoords.append(QVector2D(-0.75f, 0.75f));
      netCoords.append(QVector2D(-0.75f, -0.75f));
      netCoords.append(QVector2D(0.75f, -0.75f));
      netCoords.append(QVector2D(0.75f, 0.0f));
      netCoords.append(QVector2D(0.0f, 0.0f));
      netCoords.append(QVector2D(0.0f, -0.2f));
      netCoords.append(QVector2D(0.55f, -0.2f));
      netCoords.append(QVector2D(0.55f, -0.55f));
      netCoords.append(QVector2D(-0.55f, -0.55f));
      netCoords.append(QVector2D(-0.55f, 0.55f));
      netCoords.append(QVector2D(0.55f, 0.55f));
      netCoords.append(QVector2D(0.55f, 0.35f));
      break;
  }
  netNormals = calcNormals(netCoords);
}

/**
 * @brief SubdivisionCurve::addPoint Adds a point to the control net.
 * @param p The point to add to the control net.
 */
void SubdivisionCurve::addPoint(QVector2D p) {
  netCoords.append(p);
  reSubdivide();
}

/**
 * @brief SubdivisionCurve::setPointPosition Changes the position of a point in
 * the control net.
 * @param idx The index of the point to update the position of.
 * @param p The new position of the point.
 */
void SubdivisionCurve::setPointPosition(int idx, QVector2D p) {
  netCoords[idx] = p;
  reSubdivide();
}

/**
 * @brief SubdivisionCurve::removePoint Removes a point from the control net.
 * @param idx The index of the point to remove.
 */
void SubdivisionCurve::removePoint(int idx) {
  netCoords.remove(idx);
  reSubdivide();
}

/**
 * @brief SubdivisionCurve::findClosest Finds the index of the closest point in
 * the control net to the provided point.
 * @param p The point to find the closest point to.
 * @param maxDist The maximum distance a point and the provided point can have.
 * Is a value between 0 and 1.
 * @return The index of the closest point to the provided point. Returns -1 if
 * no point was found within the maximum distance.
 */
int SubdivisionCurve::findClosest(const QVector2D& p, const float maxDist) {
  int ptIndex = -1;
  float currentDist, minDist = 4;

  for (int k = 0; k < netCoords.size(); k++) {
    currentDist = netCoords[k].distanceToPoint(p);
    if (currentDist < minDist) {
      minDist = currentDist;
      ptIndex = k;
    }
  }
  if (minDist >= maxDist) {
    return -1;
  }

  return ptIndex;
}

void SubdivisionCurve::reSubdivide() {
  netNormals = calcNormals(netCoords);
  subdivide(subdivisionLevel);
}

/**
 * @brief SubdivisionCurve::subdivide Subdivides the curve a number of times.
 * @param level The number of times the curve should be subdivided
 */
void SubdivisionCurve::subdivide(int level) {
  if (level > subdivisionLevel && subdivisionLevel > 0) {
    // don't start subdividing from the start
    // if the previous subdivision level was 3 and the new one is 4, then we
    // only need to do a single subdividision step, instead of starting from
    // scratch and doing 4 steps.
    subdivide(curveCoords, curveNormals, level - subdivisionLevel);
  } else {
    subdivide(netCoords, netNormals, level);
  }
  subdivisionLevel = level;
}

/**
 * @brief SubdivisionCurve::subdivide Recursive subdivision function. Subdivides
 * the provided points recursively a number of times according to the
 * subdivision mask.
 * @param points The points to be subdivided
 * @param level The number of times the points should be subdivided
 */
void SubdivisionCurve::subdivide(const QVector<QVector2D>& points,
                                 const QVector<QVector2D>& normals, int level) {
  // base case
  if (level == 0) {
    curveCoords = points;
    curveNormals = normals;
    return;
  }
  int n = points.size() * 2 - 1;
  if (settings->closed) {
    n += 1;
  }
  QVector<QVector2D> newPoints(n);
  QVector<QVector2D> newNormals(n);

  // set vertex points
  for (int i = 0; i < n; i += 2) {
    newPoints[i] = points[i / 2];
    newNormals[i] = normals[i / 2];
  }
  // set new points
  int patchSize = 6;
  QVector<int> indices(patchSize);
  for (int i = 1; i < n; i += 2) {
    QVector<QVector2D> patchCoords;
    QVector<QVector2D> patchNormals;

    int pIdx = i / 2;

    int size = points.size();

    indices.clear();
    indices.append(pIdx);
    // p_2-p_1-p0-p1-p2-p3
    if (settings->closed) {
      indices.append((pIdx + 1) % size);
      indices.append((pIdx + 2) % size);
      indices.append((pIdx - 1 + size) % size);
      indices.append((pIdx + 3) % size);
      indices.append((pIdx - 2 + size) % size);
    } else {
      if (pIdx + 1 < points.size()) {
        indices.append(pIdx + 1);
        if (pIdx + 2 < points.size()) {
          indices.append(pIdx + 2);
        }
      }
      if (pIdx - 1 >= 0) {
        indices.append(pIdx - 1);
        if (pIdx - 2 >= 0) {
          indices.append(pIdx - 2);
        }
      }

      if (pIdx + 3 < points.size()) {
        indices.append(pIdx + 3);
      }
    }
    //    indices[0] = pIdx;                              // p0
    //    indices[1] = MIN(pIdx + 1, points.size() - 1);  // p1
    //    indices[2] = MIN(pIdx + 2, points.size() - 1);  // p2
    //    indices[3] = MAX(pIdx - 1, 0);                  // p_1
    //    indices[4] = MIN(pIdx + 3, points.size() - 1);  // p3
    //    indices[5] = MAX(pIdx - 2, 0);                  // p_2

    for (int j = 0; j < indices.size(); j++) {
      patchCoords.append(points[indices[j]]);
      patchNormals.append(normals[indices[j]]);
    }

    Conic conic(patchCoords, patchNormals, *settings);

    int prevIdx = (i - 1 + n) % n;
    int nextIdx = (i + 1) % n;

    const QVector2D origin = (newPoints[prevIdx] + newPoints[nextIdx]) / 2.0f;
    QVector2D dir;
    if (settings->edgeTangentSample) {
      dir = newPoints[prevIdx] - newPoints[nextIdx];
      dir.normalize();
      dir = {-dir.y(), dir.x()};
    } else {
      dir = (newNormals[prevIdx] + newNormals[nextIdx]).normalized();
    }
    QVector2D sampledPoint;
    QVector2D sampledNormal;
    const bool valid = conic.sample(origin, dir, sampledPoint, sampledNormal);

    if (!valid) {
      sampledPoint = origin;
      sampledNormal = dir;
    }

    newPoints[i] = sampledPoint;
    newNormals[i] = sampledNormal;
  }
  if (settings->recalculateNormals) {
    newNormals = calcNormals(newPoints);
  }
  subdivide(newPoints, newNormals, level - 1);
}
