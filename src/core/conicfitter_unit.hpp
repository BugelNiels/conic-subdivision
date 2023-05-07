#pragma once

#include <math.h>

#include <QVector2D>
#include <QVector>
#include <armadillo>

#include "conic.hpp"
#include "settings.hpp"

class UnitConicFitter {
public:
    UnitConicFitter();

    QVector<double> fitConic(const QVector<QVector2D> &coords,
                             const QVector<QVector2D> &normals,
                             const Settings &settings);

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

    QVector<double> vecToQVec(const arma::vec &res) const;

    QVector<double> solveLinSystem(const arma::mat &A) const;

private:
    QVector<double> solveLinSystem(const arma::mat &A, const arma::mat &B) const;

    QVector<double> fitQuadricConstrained(
            const QVector<QVector2D> &coords,
            const QVector<QVector2D> &normals) const;

    arma::mat initA(const QVector<QVector2D> &coords) const;

    arma::vec initB(const QVector<QVector2D> &normals) const;

    arma::mat initC(const QVector<QVector2D> &coords, int numConstraints) const;

    arma::mat initC(const QVector<QVector2D> &coords,
                    const QVector<QVector2D> &normals, int numConstraints) const;
};
