#include "curvesaver.hpp"

#include <QDebug>
#include <QFile>
#include <QMap>
#include <QTextStream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

namespace conics::core {

CurveSaver::CurveSaver() {}

void saveCurvetoFile(const QString &filePath, const Curve &curve) {}

bool CurveSaver::saveCurve(const char *fileName, const Curve &curve) const {
    std::ofstream file;
    file.open(fileName);

    int prec = 16;

    if (!file.is_open()) {
        qDebug() << "Failed to open file:" << fileName;
        return false;
    } else {
        //        file << "Curve data: x and y coordinates only" << std::endl;

        std::vector<Vector2DD> coords;
        coords = curve.getCoords();

        for (int i = 0; i < coords.size(); i++) {
            file << std::fixed << std::setprecision(prec) << coords[i].x() << " " << std::fixed
                 << std::setprecision(prec) << coords[i].y() << std::endl;
        }
        if (curve.isClosed()) {
            file << std::fixed << std::setprecision(prec) << coords[0].x() << " " << std::fixed
                 << std::setprecision(prec) << coords[0].y() << std::endl;
        }
    }
    file.close();
    return true;
}

bool CurveSaver::saveCurveWithNormals(const char *fileName, const Curve &curve) const {
    std::ofstream file;
    file.open(fileName);

    int prec = 16;

    if (!file.is_open()) {
        qDebug() << "Failed to open file:" << fileName;
        return false;
    } else {
        std::vector<Vector2DD> coords;
        std::vector<Vector2DD> normals;
        coords = curve.getCoords();
        normals = curve.getNormals();

        for (int i = 0; i < coords.size(); i++) {
            file << "v " << std::fixed << std::setprecision(prec) << coords[i].x() << " "
                 << std::fixed << std::setprecision(prec) << coords[i].y() << std::endl;
        }
        for (int i = 0; i < normals.size(); i++) {
            file << "vn " << std::fixed << std::setprecision(prec) << normals[i].x() << " "
                 << std::fixed << std::setprecision(prec) << normals[i].y() << std::endl;
        }
    }
    file.close();
    return true;
}

} // namespace conics::core