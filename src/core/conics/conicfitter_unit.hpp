#pragma once

#include <QVector>

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
    int numPoints_ = 0;
    int numNormals_ = 0;
    int numConstraints_ = 0;
    int numEq_ = 0;
    double pointWeight_ = 1.0;
    double normalWeight_ = 1.0;
    double middlePointWeight_ = 1.0;
    double middleNormalWeight_ = 1.0;
    int numUnknowns_ = 0;

    double getPointWeight(int index) const;

    double getNormalWeight(int index) const;

    QVector<double> fitQuadricConstrained(
            const QVector<Vector2DD> &coords,
            const QVector<Vector2DD> &normals) const;

};
