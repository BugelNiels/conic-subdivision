#include "conis/core/curve/curveloader.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace conis::core {

CurveLoader::CurveLoader() {}

Curve CurveLoader::loadCurveFromFile(const std::string &filePath) {
    // Note that std::stod is dependent on the Locale
    std::setlocale(LC_NUMERIC, "C");
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    std::vector<Vector2DD> verts, normals;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string part;
        std::vector<std::string> parts;

        while (lineStream >> part) {
            parts.push_back(part);
        }
        if (parts.empty())
            continue;
        const std::string type = parts[0];
        if (type == "v") {
            if (parts.size() >= 3) {
                Vector2DD vertex;
                vertex.x() = std::stod(parts[1]);
                vertex.y() = std::stod(parts[2]);
                verts.emplace_back(vertex);
            }
        } else if (type == "vn") {
            if (parts.size() >= 3) {
                Vector2DD normal;
                normal.x() = std::stod(parts[1]);
                normal.y() = std::stod(parts[2]);
                normals.emplace_back(normal.normalized());
            }
        }
    }

    bool closed = false;

    file.close();
    std::cout << "Loaded curve with " << verts.size() << " vertices and " << normals.size() << " normals" << std::endl;

    if (normals.empty()) {
        return Curve(verts, closed);
    }
    return {verts, normals, closed};
}

} // namespace conis::core
