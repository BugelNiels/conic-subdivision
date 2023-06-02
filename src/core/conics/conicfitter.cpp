#include "conicfitter.hpp"
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <Eigen/Eigen>

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

inline Eigen::RowVectorXd pointEqEigen(const Vector2DD &coord, int numUnknowns) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    double x = coord.x();
    double y = coord.y();
    row(0) = x * x;
    row(1) = y * y;
    row(2) = 2 * x * y;
    row(3) = 2 * x;
    row(4) = 2 * y;
    row(5) = 1;
    return row;
}

inline Eigen::RowVectorXd normEqXEigen(const Vector2DD &coord, const Vector2DD &normal,
                                       int numUnknowns, int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    double x = coord.x();
    double y = coord.y();
    row(0) = 2 * x;  // A
    row(1) = 0;      // B
    row(2) = 2 * y;  // C
    row(3) = 2;      // D
    row(4) = 0;      // E
    row(5) = 0;      // F
    row(6 + normIdx) = -normal.x();
    return row;
}

inline Eigen::RowVectorXd normEqYEigen(const Vector2DD &coord, const Vector2DD &normal,
                                       int numUnknowns, int normIdx) {
    Eigen::RowVectorXd row = Eigen::RowVectorXd::Zero(numUnknowns);
    double x = coord.x();
    double y = coord.y();
    row(0) = 0;        // A
    row(1) = 2 * y;    // B
    row(2) = 2 * x;    // C
    row(3) = 0;        // E
    row(4) = 2;        // F
    row(5) = 0;        // G
    row(6 + normIdx) = -normal.y();
    return row;
}

Eigen::MatrixXd ConicFitter::initAEigen(const QVector<Vector2DD> &coords,
                                        const QVector<Vector2DD> &normals) const {
    Eigen::MatrixXd A(numEq_, numUnknowns_);

    int rowIdx = 0;
    for (int i = 0; i < numPoints_; i++) {
        double weight = getPointWeight(i);
        A.row(rowIdx++) = pointEqEigen(coords[i], numUnknowns_) * weight;
    }
    for (int i = 0; i < numNormals_; i++) {
        double weight = getNormalWeight(i);
        const Vector2DD &coord = coords[i];
        const Vector2DD &normal = normals[i];
        A.row(rowIdx++) = normEqXEigen(coord, normal, numUnknowns_, i) * weight;
        A.row(rowIdx++) = normEqYEigen(coord, normal, numUnknowns_, i) * weight;
    }
    return A;
}

Eigen::VectorXd ConicFitter::solveLinSystem(const Eigen::MatrixXd &A) {
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
    stability_ = svd.singularValues()(0) / svd.singularValues()(svd.singularValues().size() - 1);
    return V.col(V.cols() - 1);
#endif
}

Eigen::VectorXd ConicFitter::fitConic(const QVector<Vector2DD> &coords,
                                      const QVector<Vector2DD> &normals) {
    numPoints_ = int(coords.size());
    numNormals_ = int(normals.size());
    numUnknowns_ = 6 + numNormals_;

    pointWeight_ = settings_.pointWeight;
    normalWeight_ = settings_.normalWeight;
    middlePointWeight_ = settings_.middlePointWeight;
    middleNormalWeight_ = settings_.middleNormalWeight;

    numEq_ = numPoints_ + numNormals_ * 2;
    return solveLinSystem(initAEigen(coords, normals));
}

double ConicFitter::stability() {
    double minVal = 10.0 * std::max(settings_.pointWeight, settings_.normalWeight);
    double maxVal = 1.0e9 * minVal;
    double logMin = std::log10(minVal);
    double logMax = std::log10(maxVal);

    double logValue = std::log10(stability_);

    double mappedValue = (logValue - logMin) / (logMax - logMin);
    return std::clamp(mappedValue, 0.0, 1.0);
}
