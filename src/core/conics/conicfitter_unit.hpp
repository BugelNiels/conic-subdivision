#pragma once

#include <QVector>
#include <armadillo>

#include "src/core/settings.hpp"
#include "util/vector.hpp"

class UnitConicFitter {
public:
    UnitConicFitter();

    QVector<double> fitConic(const QVector<Vector2DD> &coords,
                             const QVector<Vector2DD> &normals,
                             const Settings &settings);

    double stability() const;

private:
    double stability_ = 0;
    int numPoints_;
    int numNormals_;
    int numConstraints_;
    int numEq_;
    double pointWeight_ = 1.0;
    double normalWeight_ = 1.0;
    double middlePointWeight_ = 1.0;
    double middleNormalWeight_ = 1.0;
    int numUnknowns_;

    double getPointWeight(int index) const;

    double getNormalWeight(int index) const;

    QVector<double> vecToQVec(const arma::vec &res) const;

    QVector<double> solveLinSystem(const arma::mat &A, const arma::mat &B) const;

    QVector<double> fitQuadricConstrained(
            const QVector<Vector2DD> &coords,
            const QVector<Vector2DD> &normals) const;

    arma::mat initA(const QVector<Vector2DD> &coords) const;

    arma::vec initB(const QVector<Vector2DD> &normals) const;

    arma::mat initC(const QVector<Vector2DD> &coords, int numConstraints) const;

    arma::mat initC(const QVector<Vector2DD> &coords,
                    const QVector<Vector2DD> &normals, int numConstraints) const;
};
