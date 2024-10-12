#include "curvesaver.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

namespace conis::core {

CurveSaver::CurveSaver() {}

bool CurveSaver::saveCurve(const std::string &fileName, const Curve &curve) const {
    std::ofstream file(fileName);
    const int prec = 16; // Set precision for output

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }

    // Get coordinates from curve
    const std::vector<Vector2DD> &verts = curve.getVertices();

    // Write coordinates to the file
    for (const auto &coord: verts) {
        file << std::fixed << std::setprecision(prec) << coord.x() << " " << std::fixed << std::setprecision(prec)
             << coord.y() << std::endl;
    }

    // If curve is closed, write the first coordinate again
    if (curve.isClosed()) {
        file << std::fixed << std::setprecision(prec) << verts[0].x() << " " << std::fixed << std::setprecision(prec)
             << verts[0].y() << std::endl;
    }

    file.close();
    return true;
}

bool CurveSaver::saveCurveWithNormals(const std::string &fileName, const Curve &curve) const {
    std::ofstream file(fileName);
    const int prec = 16; // Set precision for output

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }

    // Get coordinates and normals from curve
    const std::vector<Vector2DD> &verts = curve.getVertices();
    const std::vector<Vector2DD> &normals = curve.getNormals();

    // Write coordinates to the file with "v" prefix
    for (const auto &coord: verts) {
        file << "v " << std::fixed << std::setprecision(prec) << coord.x() << " " << std::fixed
             << std::setprecision(prec) << coord.y() << std::endl;
    }

    // Write normals to the file with "vn" prefix
    for (const auto &normal: normals) {
        file << "vn " << std::fixed << std::setprecision(prec) << normal.x() << " " << std::fixed
             << std::setprecision(prec) << normal.y() << std::endl;
    }

    file.close();
    return true;
}

} // namespace conis::core
