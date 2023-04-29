#ifndef CONICFITTER_H
#define CONICFITTER_H

#include <math.h>

#include <QVector2D>
#include <QVector>
#include <armadillo>

#include "conic.h"
#include "settings.h"

class ConicFitter {
 public:
  ConicFitter();

  QVector<double> fitConic(const QVector<QVector2D>& coords,
                           const QVector<QVector2D>& normals,
                           const Settings& settings);

 private:
  int numPoints;
  int numNormals;
  int numConstraints;
  int numEq;
  double pointWeight = 1.0;
  double normalWeight = 1.0;
  double middlePointWeight = 1.0;
  double middleNormalWeight = 1.0;
  double outerPointWeight = 1.0;
  double outerNormalWeight = 1.0;
  int numUnkowns;

  double getPointWeight(int index) const;
  double getNormalWeight(int index) const;
  QVector<double> vecToQVec(const arma::vec& res) const;
  QVector<double> solveLinSystem(const arma::mat& A) const;

 private:
  arma::mat initA(const QVector<QVector2D>& coords,
                  const QVector<QVector2D>& normals) const;
};

#endif  // CONICFITTER_H
