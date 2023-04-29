#include "conicfitter_unit.h"

using namespace arma;

UnitConicFitter::UnitConicFitter() {}

double UnitConicFitter::getPointWeight(int index) const {
  if (index < 2) {
    return pointWeight;
  } else if (index < 4) {
    return middlePointWeight;
  }
  return outerPointWeight;
}

double UnitConicFitter::getNormalWeight(int index) const {
  if (index < 2) {
    return normalWeight;
  } else if (index < 4) {
    return middleNormalWeight;
  }
  return outerNormalWeight;
}

inline rowvec pointEq(const QVector2D& coord, int numUnkowns) {
  rowvec row(numUnkowns);
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

inline rowvec normEqX(const QVector2D& coord, int numUnkowns) {
  rowvec row(numUnkowns);
  double x = coord.x();
  double y = coord.y();
  row(0) = 2 * x;  // A
  row(1) = 0;      // B
  row(2) = y;      // C
  row(3) = 1;      // D
  row(4) = 0;      // E
  row(5) = 0;      // F
  return row;
}

inline rowvec normEqY(const QVector2D& coord, int numUnkowns) {
  rowvec row(numUnkowns);
  double x = coord.x();
  double y = coord.y();
  row(0) = 0;        // A
  row(1) = (2 * y);  // B
  row(2) = x;        // C
  row(3) = 0;        // E
  row(4) = 1;        // F
  row(5) = 0;        // G
  return row;
}

mat UnitConicFitter::initA(const QVector<QVector2D>& coords) const {
  mat A(numEq, numUnkowns);

  uword rowIdx = 0;
  for (int i = 0; i < numPoints; i++) {
    float weight = getPointWeight(i);
    A.row(rowIdx++) = pointEq(coords[i], numUnkowns) * weight;
  }
  for (int i = 0; i < numNormals; i++) {
    float weight = getNormalWeight(i);
    QVector2D coord = coords[i];
    A.row(rowIdx++) = normEqX(coord, numUnkowns) * weight;
    A.row(rowIdx++) = normEqY(coord, numUnkowns) * weight;
  }
  return A;
}

/**
 * @brief QuadricFitter::initB Inits the vector B.
 * @param normals The expected normals for the normal equations.
 * @return Vector B.
 */
vec UnitConicFitter::initB(const QVector<QVector2D>& normals) const {
  vec B = zeros(numEq);
  uword eqIdx = numPoints;
  for (int i = 0; i < numNormals; i++) {
    float weight = getNormalWeight(i);
    B(eqIdx++) = normals[i].x() * weight;
    B(eqIdx++) = normals[i].y() * weight;
  }
  return B;
}

mat UnitConicFitter::initC(const QVector<QVector2D>& coords,
                           const QVector<QVector2D>& normals,
                           int numConstraints) const {
  mat C(3 * numConstraints, numUnkowns);

  uword rowIdx = 0;
  for (int i = 0; i < numConstraints; i++) {
    C.row(rowIdx++) = pointEq(coords[i], numUnkowns);
    C.row(rowIdx++) = normEqX(normals[i], numUnkowns);
    C.row(rowIdx++) = normEqY(normals[i], numUnkowns);
  }
  return C;
}

mat UnitConicFitter::initC(const QVector<QVector2D>& coords,
                           int numConstraints) const {
  mat C(numConstraints, numUnkowns);

  uword rowIdx = 0;
  for (int i = 0; i < numConstraints; i++) {
    C.row(rowIdx++) = pointEq(coords[i], numUnkowns);
  }
  return C;
}

/**
 * @brief QuadricFitter::vecToQVec Convert a result arma::vec to a QVector.
 * @param res The vector to convert.
 * @return A QVector containing the result. If the input vector contained only
 * zeros, an empty vector is returned.
 */
QVector<double> UnitConicFitter::vecToQVec(const vec& res) const {
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

QVector<double> UnitConicFitter::solveLinSystem(const mat& A,
                                                const mat& B) const {
  vec result;

  bool hasSolution;
  //#pragma omp critical
  hasSolution = solve(result, A, B, solve_opts::no_approx);

  if (hasSolution) {
    return vecToQVec(result);
  }
  return QVector<double>();
}

/**
 * @brief QuadricFitter::fitQuadricConstrained Fits a quadric by approximating
 * both the the normals and fitting the points exactly.
 * @param patch The patch to fit the quadric to.
 * @return The quadric coefficients. Empty if no quadric was found.
 */
QVector<double> UnitConicFitter::fitQuadricConstrained(
    const QVector<QVector2D>& coords, const QVector<QVector2D>& normals) const {
  mat A = initA(coords);
  vec B = initB(normals);
#if 0
  mat C = initC(coords, normals, numConstraints);
  vec D = zeros(3 * numConstraints);
  uword idx = 0;
  for (int i = 0; i < numConstraints; i++) {
    D(idx++) = 0;
    D(idx++) = normals[i].x();
    D(idx++) = normals[i].y();
  }
//  vec D = zeros(numConstraints * 3);
#else

  mat C = initC(coords, numConstraints);
  vec D = zeros(C.n_rows);
#endif

  uword quarterDim = A.n_cols;
  uword totalDim = quarterDim + C.n_rows;

  mat Q = zeros(totalDim, totalDim);
  Q.submat(0, 0, quarterDim - 1, quarterDim - 1) = A.t() * A;
  Q.submat(quarterDim, 0, totalDim - 1, quarterDim - 1) = C;
  Q.submat(0, quarterDim, quarterDim - 1, totalDim - 1) = C.t();

  vec eqTo(totalDim);
  eqTo.subvec(0, quarterDim - 1) = A.t() * B;
  eqTo.subvec(quarterDim, totalDim - 1) = D;

  return solveLinSystem(Q, eqTo);
}

QVector<double> UnitConicFitter::fitConic(const QVector<QVector2D>& coords,
                                          const QVector<QVector2D>& normals,
                                          const Settings& settings) {
  numPoints = coords.size();
  numNormals = normals.size();
  pointWeight = settings.pointWeight;
  normalWeight = settings.normalWeight;
  middlePointWeight = settings.middlePointWeight;
  middleNormalWeight = settings.middleNormalWeight;
  outerPointWeight = settings.outerPointWeight;
  outerNormalWeight = settings.outerNormalWeight;

  numEq = numPoints + numNormals * 2;
  numConstraints = 2;
  numUnkowns = 6;

  return fitQuadricConstrained(coords, normals);
}
