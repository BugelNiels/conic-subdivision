#include "conicfitter.hpp"
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Eigen/Eigen>

ConicFitter::ConicFitter() {}



double ConicFitter::getPointWeight(int index) const {
    if (index < 2) {
        return pointWeight_;
    } else if (index < 4) {
        return middlePointWeight_;
    }
    return outerPointWeight_;
}

double ConicFitter::getNormalWeight(int index) const {
    if (index < 2) {
        return normalWeight_;
    } else if (index < 4) {
        return middleNormalWeight_;
    }
    return outerNormalWeight_;
}


inline arma::rowvec pointEq(const QVector2D &coord, int numUnkowns) {
    arma::rowvec row = arma::zeros(1, numUnkowns);
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

inline arma::rowvec normEqX(const QVector2D &coord, const QVector2D &normal,
                            int numUnkowns, int normIdx) {
    arma::rowvec row = arma::zeros(1, numUnkowns);
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

inline arma::rowvec normEqY(const QVector2D &coord, const QVector2D &normal,
                            int numUnkowns, int normIdx) {
    arma::rowvec row = arma::zeros(1, numUnkowns);
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

arma::mat ConicFitter::initA(const QVector<QVector2D> &coords,
                             const QVector<QVector2D> &normals) const {
    arma::mat A(numEq_, numUnknowns_);

    arma::uword rowIdx = 0;
    for (int i = 0; i < numPoints_; i++) {
        double weight = getPointWeight(i);
        A.row(rowIdx++) = pointEq(coords[i], numUnknowns_) * weight;
    }
    for (int i = 0; i < numNormals_; i++) {
        double weight = getNormalWeight(i);
        QVector2D coord = coords[i];
        QVector2D normal = normals[i];
        A.row(rowIdx++) = normEqX(coord, normal, numUnknowns_, i) * weight;
        A.row(rowIdx++) = normEqY(coord, normal, numUnknowns_, i) * weight;
    }
    return A;
}

QVector<double> ConicFitter::vecToQVec(const arma::vec &res) const {
    QVector<double> coefs;
    int numZeros = 0;
    for (int i = 0; i < numUnknowns_; i++) {
        coefs.append(res(i));
        if (res(i) == 0.0) {
            numZeros++;
        }
    }
    if (numZeros == numUnknowns_) {
        return QVector<double>();
    }
    return coefs;
}

QVector<double> ConicFitter::solveLinSystem(const arma::mat &A) const {
    arma::mat U;
    arma::vec S;
    arma::mat V;

    qDebug() << arma::cond(A);

#pragma omp critical
    bool hasSolution = svd(U, S, V, A);

    if (hasSolution) {
        return vecToQVec(V.col(V.n_cols - 1));
    }
    return QVector<double>();
}



inline Eigen::RowVectorXd pointEqEigen(const QVector2D &coord, int numUnkowns) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnkowns);
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

inline Eigen::RowVectorXd normEqXEigen(const QVector2D &coord, const QVector2D &normal,
                                  int numUnkowns, int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnkowns);
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

inline Eigen::RowVectorXd normEqYEigen(const QVector2D &coord, const QVector2D &normal,
                                  int numUnkowns, int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnkowns);
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

Eigen::MatrixXd ConicFitter::initAEigen(const QVector<QVector2D> &coords,
                                   const QVector<QVector2D> &normals) const {
    Eigen::MatrixXd A(numEq_, numUnknowns_);

    int rowIdx = 0;
    for (int i = 0; i < numPoints_; i++) {
        double weight = getPointWeight(i);
        A.row(rowIdx++) = pointEqEigen(coords[i], numUnknowns_) * weight;
    }
    for (int i = 0; i < numNormals_; i++) {
        double weight = getNormalWeight(i);
        QVector2D coord = coords[i];
        QVector2D normal = normals[i];
        A.row(rowIdx++) = normEqXEigen(coord, normal, numUnknowns_, i) * weight;
        A.row(rowIdx++) = normEqYEigen(coord, normal, numUnknowns_, i) * weight;
    }
    return A;
}

QVector<double> ConicFitter::vecToQVecEigen(const Eigen::VectorXd &res) const {
    QVector<double> coefs;
    int numZeros = 0;
    for (int i = 0; i < numUnknowns_; i++) {
        coefs.append(res(i));
        if (res(i) == 0.0) {
            numZeros++;
        }
    }
    if (numZeros == numUnknowns_) {
        return QVector<double>();
    }
    return coefs;
}

QVector<double> ConicFitter::solveLinSystem(const Eigen::MatrixXd &A) const {
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullV);
    const auto& V = svd.matrixV();
    Eigen::VectorXd eigenVec = V.col(V.cols() - 1);

    if (svd.singularValues()(V.cols() - 1) > 1e-12) {
        return vecToQVecEigen(eigenVec);
    }
    return QVector<double>();
}

QVector<double> ConicFitter::fitConic(const QVector<QVector2D> &coords,
                                      const QVector<QVector2D> &normals,
                                      const Settings &settings) {
    numPoints_ = coords.size();
    numNormals_ = normals.size();
    if (settings.outerNormalWeight == 0.0 || settings.outerPointWeight == 0.0) {
        numNormals_ -= 2;
        numPoints_ -= 2;
    }
    numUnknowns_ = 6 + numNormals_;

    pointWeight_ = settings.pointWeight;
    normalWeight_ = settings.normalWeight;
    middlePointWeight_ = settings.middlePointWeight;
    middleNormalWeight_ = settings.middleNormalWeight;
    outerPointWeight_ = settings.outerPointWeight;
    outerNormalWeight_ = settings.outerNormalWeight;

    numEq_ = numPoints_ + numNormals_ * 2;
#ifdef ARMADILLO
    return solveLinSystem(initA(coords, normals));
#else
    return solveLinSystem(initAEigen(coords, normals));
#endif
}
