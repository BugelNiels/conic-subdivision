#include "conicfitter.h"

using namespace arma;

ConicFitter::ConicFitter() {}

inline rowvec pointEq(const QVector2D& coord, int numUnkowns) {
  rowvec row = zeros(1, numUnkowns);
  double x = coord.x();
  double y = coord.y();
  row(0) = x * x;
  row(1) = y * y;
  row(2) = x * y;
  row(3) = x;
  row(4) = y;
  row(5) = 1;
  return row;
}

inline rowvec normEqX(const QVector2D& coord, const QVector2D& normal,
                      int numUnkowns, int normIdx) {
  rowvec row = zeros(1, numUnkowns);
  double x = coord.x();
  double y = coord.y();
  row(0) = 2 * x;  // A
  row(1) = 0;      // B
  row(2) = y;      // C
  row(3) = 1;      // D
  row(4) = 0;      // E
  row(5) = 0;      // F
  row(6 + normIdx) = -normal.x();
  return row;
}

inline rowvec normEqY(const QVector2D& coord, const QVector2D& normal,
                      int numUnkowns, int normIdx) {
  rowvec row = zeros(1, numUnkowns);
  double x = coord.x();
  double y = coord.y();
  row(0) = 0;        // A
  row(1) = (2 * y);  // B
  row(2) = x;        // C
  row(3) = 0;        // E
  row(4) = 1;        // F
  row(5) = 0;        // G
  row(6 + normIdx) = -normal.y();
  return row;
}

double ConicFitter::getPointWeight(int index) const {
  if (index < 2) {
    return pointWeight;
  } else if (index < 4) {
    return middlePointWeight;
  }
  return outerPointWeight;
}

double ConicFitter::getNormalWeight(int index) const {
  if (index < 2) {
    return normalWeight;
  } else if (index < 4) {
    return middleNormalWeight;
  }
  return outerNormalWeight;
}

mat ConicFitter::initA(const QVector<QVector2D>& coords,
                       const QVector<QVector2D>& normals) const {
  mat A(numEq, numUnkowns);

  uword rowIdx = 0;
  for (int i = 0; i < numPoints; i++) {
    double weight = getPointWeight(i);
    A.row(rowIdx++) = pointEq(coords[i], numUnkowns) * weight;
  }
  for (int i = 0; i < numNormals; i++) {
    double weight = getNormalWeight(i);
    QVector2D coord = coords[i];
    QVector2D normal = normals[i];
    A.row(rowIdx++) = normEqX(coord, normal, numUnkowns, i) * weight;
    A.row(rowIdx++) = normEqY(coord, normal, numUnkowns, i) * weight;
  }
  return A;
}

QVector<double> ConicFitter::vecToQVec(const vec& res) const {
  QVector<double> coefs;
  int numZeros = 0;
  for (int i = 0; i < numUnkowns; i++) {
    coefs.append(res(i));
    if (res(i) == 0.0) {
      numZeros++;
    }
  }
  if (numZeros == numUnkowns) {
    return QVector<double>();
  }
  return coefs;
}

QVector<double> ConicFitter::solveLinSystem(const mat& A) const {
  bool hasSolution;

  mat U;
  vec S;
  mat V;
#pragma omp critical
  hasSolution = svd(U, S, V, A);

  if (hasSolution) {
    return vecToQVec(V.col(V.n_cols - 1));
  }
  return QVector<double>();
}

QVector<double> ConicFitter::fitConic(const QVector<QVector2D>& coords,
                                      const QVector<QVector2D>& normals,
                                      const Settings& settings) {
  numPoints = coords.size();
  numNormals = normals.size();
  if (settings.outerNormalWeight == 0.0 || settings.outerPointWeight == 0.0) {
    numNormals -= 2;
    numPoints -= 2;
  }
  numUnkowns = 6 + numNormals;

  pointWeight = settings.pointWeight;
  normalWeight = settings.normalWeight;
  middlePointWeight = settings.middlePointWeight;
  middleNormalWeight = settings.middleNormalWeight;
  outerPointWeight = settings.outerPointWeight;
  outerNormalWeight = settings.outerNormalWeight;

  numEq = numPoints + numNormals * 2;

  return solveLinSystem(initA(coords, normals));
}
