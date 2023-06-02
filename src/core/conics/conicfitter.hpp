#pragma once

#include <QVector>

#include "src/core/settings.hpp"
#include <Eigen/Core>
#include "util/vector.hpp"

class ConicFitter {
public:
    explicit ConicFitter(const Settings &settings);

    Eigen::VectorXd fitConic(const QVector<Vector2DD> &coords,
                             const QVector<Vector2DD> &normals);

    double stability();

private:
    const Settings &settings_;
    double stability_ = 0;
    int numPoints_ = 0;
    int numNormals_ = 0;
    int numEq_ = 0;

    double pointWeight_ = 1.0;
    double normalWeight_ = 1.0;
    double middlePointWeight_ = 1.0;
    double middleNormalWeight_ = 1.0;
    int numUnknowns_ = 0;

    double getPointWeight(int index) const;

    double getNormalWeight(int index) const;

    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>
    initAEigen(const QVector<Vector2DD> &coords, const QVector<Vector2DD> &normals) const;

    Eigen::VectorXd solveLinSystem(const Eigen::MatrixXd &A);
};