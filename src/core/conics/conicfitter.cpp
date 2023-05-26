#include "conicfitter.hpp"
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Eigen/Eigen>

//#define ARMADILLO

ConicFitter::ConicFitter(const Settings &settings) : settings_(settings) {}


double ConicFitter::getPointWeight(int index) const {
    if (index < 2) {
        return pointWeight_;
    } else if (index < 4) {
        return middlePointWeight_;
    }
    return 0;
}

double ConicFitter::getNormalWeight(int index) const {
    if (index < 2) {
        return normalWeight_;
    } else if (index < 4) {
        return middleNormalWeight_;
    }
    return 0;
}


inline arma::rowvec pointEq(const QVector2D &coord, int numUnknowns) {
    arma::rowvec row = arma::zeros(1, numUnknowns);
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
    QVector<double> coefficients;
    int numZeros = 0;
    for (int i = 0; i < numUnknowns_; i++) {
        coefficients.append(res(i));
        if (res(i) == 0.0) {
            numZeros++;
        }
    }
    if (numZeros == numUnknowns_) {
        return {};
    }
    return coefficients;
}

QVector<double> ConicFitter::solveLinSystem(const arma::mat &A) const {
    arma::mat U;
    arma::vec S;
    arma::mat V;

    bool hasSolution = svd(U, S, V, A);
    if (hasSolution) {
        return vecToQVec(V.col(V.n_cols - 1));
    }
    return {};
}


inline Eigen::RowVectorXd pointEqEigen(const QVector2D &coord, int numUnknowns) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
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
    QVector<double> coefficients;
    int numZeros = 0;
    for (int i = 0; i < numUnknowns_; i++) {
        coefficients.append(res(i));
        if (res(i) == 0.0) {
            numZeros++;
        }
    }
    if (numZeros == numUnknowns_) {
        return {};
    }
    return coefficients;
}

QVector<double> ConicFitter::solveLinSystem(const Eigen::MatrixXd &A) {
#if 0
    Eigen::MatrixXd MtM = A.transpose() * A;
    Eigen::EigenSolver<Eigen::MatrixXd> eigensolver(MtM);
    Eigen::VectorXd eigenvalues = eigensolver.eigenvalues().real();
    Eigen::MatrixXd eigenvectors = eigensolver.eigenvectors().real();

    int minIndex;
    double minValue = std::numeric_limits<double>::max();
    for (int i = 0; i < eigenvalues.size(); ++i) {
        double absValue = std::abs(eigenvalues(i));
        if (absValue < minValue) {
            minValue = absValue;
            minIndex = i;
        }
    }

    Eigen::VectorXd solution = eigenvectors.col(minIndex);
    return vecToQVecEigen(solution);
#else
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeThinV);
    const auto &V = svd.matrixV();
    int idx = int(V.cols() - 1);
    Eigen::VectorXd eigenVec = V.col(idx);
    stability_ = float(svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size() - 1));
    if (svd.singularValues()(idx) > 1e-20) {
        return vecToQVecEigen(eigenVec);
    }

    return {};
#endif
}

QVector<double> ConicFitter::fitConic(const QVector<QVector2D> &coords,
                                      const QVector<QVector2D> &normals) {
    numPoints_ = int(coords.size());
    numNormals_ = int(normals.size());
    numUnknowns_ = 6 + numNormals_;

    pointWeight_ = settings_.pointWeight;
    normalWeight_ = settings_.normalWeight;
    middlePointWeight_ = settings_.middlePointWeight;
    middleNormalWeight_ = settings_.middleNormalWeight;

    numEq_ = numPoints_ + numNormals_ * 2;
#ifdef ARMADILLO
    auto A = initA(coords, normals);
    stability_ = arma::cond(A);
    return solveLinSystem(initA(coords, normals));
#else
    return solveLinSystem(initAEigen(coords, normals));
#endif
}

float ConicFitter::stability() {
    float minVal = 10.0f * float(std::max(settings_.pointWeight, settings_.normalWeight));
    float maxVal = 1.0e9f * minVal;
    float logMin = std::log10(minVal);
    float logMax = std::log10(maxVal);

    float logValue = std::log10(stability_);

    float mappedValue = (logValue - logMin) / (logMax - logMin);
    return std::clamp(mappedValue, 0.0f, 1.0f);
}
