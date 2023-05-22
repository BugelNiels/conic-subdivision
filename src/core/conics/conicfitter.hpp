#pragma once

#include <QVector2D>
#include <QVector>

#include "src/core/settings.hpp"
#include <armadillo>
#include <Eigen/Core>

class ConicFitter {
public:
    explicit ConicFitter(const Settings &settings);

    QVector<double> fitConic(const QVector<QVector2D> &coords,
                             const QVector<QVector2D> &normals);

    float stability();

private:
    const Settings &settings_;
    float stability_ = 0;
    int numPoints_ = 0;
    int numNormals_ = 0;
    int numEq_ = 0;

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

    QVector<double> solveLinSystem(const arma::mat &A) const;

    arma::mat initA(const QVector<QVector2D> &coords,
                    const QVector<QVector2D> &normals) const;

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>
    initAEigen(const QVector<QVector2D> &coords, const QVector<QVector2D> &normals) const;

    QVector<double> solveLinSystem(const Eigen::MatrixXd &A);

    QVector<double> vecToQVecEigen(const Eigen::VectorXd &res) const;
};