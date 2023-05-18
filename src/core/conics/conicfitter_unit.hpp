#pragma once

#include <QVector2D>
#include <QVector>
#include <armadillo>

#include "src/core/settings.hpp"

class UnitConicFitter {
public:
    UnitConicFitter();

    QVector<double> fitConic(const QVector<QVector2D> &coords,
                             const QVector<QVector2D> &normals,
                             const Settings &settings);

    float stability() const;

private:
    float stability_ = 0;
    int numPoints_;
    int numNormals_;
    int numConstraints_;
    int numEq_;
    double pointWeight_ = 1.0;
    double normalWeight_ = 1.0;
    double middlePointWeight_ = 1.0;
    double middleNormalWeight_ = 1.0;
    double outerPointWeight_ = 1.0;
    double outerNormalWeight_ = 1.0;
    int numUnknowns_;

    double getPointWeight(int index) const;

    double getNormalWeight(int index) const;

    QVector<double> vecToQVec(const arma::vec &res) const;

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
