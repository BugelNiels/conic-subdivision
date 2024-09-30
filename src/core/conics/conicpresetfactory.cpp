#include "conicpresetfactory.hpp"

#include "util/vector.hpp"


Curve getBlank() {
    std::vector<Vector2DD> netCoords;
    return Curve(netCoords, true);
}

Curve getLine() {
    std::vector<Vector2DD> netCoords;
    netCoords.emplace_back(-0.75, 0);
    netCoords.emplace_back(0.75, 0);
    return Curve(netCoords, false);
}

Curve getStair() {
    std::vector<Vector2DD> netCoords;
    netCoords.emplace_back(-1, 1);
    netCoords.emplace_back(1, 1);
    netCoords.emplace_back(1, -1);
    netCoords.emplace_back(0.33333, -1);
    netCoords.emplace_back(0.33333, -.33333);
    netCoords.emplace_back(-0.33333, -.33333);
    netCoords.emplace_back(-0.33333, .33333);
    netCoords.emplace_back(-1, .33333);
    return Curve(netCoords, true);
}

Curve getStar() {
    std::vector<Vector2DD> netCoords;
    int numPoints = 5;
    netCoords.reserve(numPoints * 2);
    double radius = 1;

    for (int i = 0; i < numPoints; ++i) {
        // Outer points
        double theta = i * (2.0 * M_PI / numPoints);
        double x = radius * std::cos(theta);
        double y = radius * std::sin(theta);
        netCoords.emplace_back(x, y);

        // Inner points
        theta += M_PI / numPoints;
        x = 0.3 * radius * std::cos(theta);
        y = 0.3 * radius * std::sin(theta);
        netCoords.emplace_back(x, y);
    }
    return Curve(netCoords, true);
}

Curve getPentagon() {
    std::vector<Vector2DD> netCoords;
    netCoords.reserve(5);
    netCoords.emplace_back(-0.25, -0.5);
    netCoords.emplace_back(-0.75, 0.0);
    netCoords.emplace_back(-0.25, 0.75);
    netCoords.emplace_back(0.75, 0.5);
    netCoords.emplace_back(0.5, -0.75);
    return Curve(netCoords);
}

Curve getBasis() {
    std::vector<Vector2DD> netCoords;
    netCoords.reserve(9);
    netCoords.emplace_back(-1.0, -0.25);
    netCoords.emplace_back(-0.75, -0.25);
    netCoords.emplace_back(-0.5, -0.25);
    netCoords.emplace_back(-0.25, -0.25);
    netCoords.emplace_back(0.0, 0.50);
    netCoords.emplace_back(0.25, -0.25);
    netCoords.emplace_back(0.5, -0.25);
    netCoords.emplace_back(0.75, -0.25);
    netCoords.emplace_back(1.0, -0.25);
    return Curve(netCoords, false);
}

Curve getG() {
    std::vector<Vector2DD> netCoords;
    netCoords.reserve(14);
    netCoords.emplace_back(0.75, 0.35);
    netCoords.emplace_back(0.75, 0.75);
    netCoords.emplace_back(-0.75, 0.75);
    netCoords.emplace_back(-0.75, -0.75);
    netCoords.emplace_back(0.75, -0.75);
    netCoords.emplace_back(0.75, 0.0);
    netCoords.emplace_back(0.0, 0.0);
    netCoords.emplace_back(0.0, -0.2);
    netCoords.emplace_back(0.55, -0.2);
    netCoords.emplace_back(0.55, -0.55);
    netCoords.emplace_back(-0.55, -0.55);
    netCoords.emplace_back(-0.55, 0.55);
    netCoords.emplace_back(0.55, 0.55);
    netCoords.emplace_back(0.55, 0.35);
    return Curve(netCoords);
}

Curve getCircle() {
    int numPoints = 4;
    double radius = 0.5;
    std::vector<Vector2DD> netCoords;
    std::vector<Vector2DD> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const double theta = i * (2.0 * M_PI / numPoints);
        const double x = radius * std::cos(theta);
        const double y = radius * std::sin(theta);
        netCoords.emplace_back(x, y);
        netNormals.emplace_back(Vector2DD(x, y).normalized());
    }
    return {netCoords, netNormals};
}

Curve getEllipse() {
    int numPoints = 5;
    double width = 0.8;
    double height = 0.3;
    std::vector<Vector2DD> netCoords;
    std::vector<Vector2DD> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const double theta = i * (2.0 * M_PI / numPoints);
        const double x = width * std::cos(theta);
        const double y = height * std::sin(theta);
        netCoords.emplace_back(x, y);
        const double tx = -width * std::sin(theta);
        const double ty = height * std::cos(theta);
        const Vector2DD normal(ty, -tx);
        netNormals.emplace_back(normal.normalized());
    }
    return {netCoords, netNormals};
}

Curve getJiriTest() {
    std::vector<Vector2DD> netCoords;
    std::vector<Vector2DD> netNormals;
    netCoords.emplace_back(-0.75, 0);
    netCoords.emplace_back(0.75, 0);
    netCoords.emplace_back(0.75, 0.5);

    netNormals.emplace_back(Vector2DD(0.3, 0.3).normalized());
    netNormals.emplace_back(Vector2DD(0.3, 0.3).normalized());
    netNormals.emplace_back(Vector2DD(0.3, 0.3).normalized());

    return Curve(netCoords, netNormals, false);
}


conics::ConicPresetFactory::ConicPresetFactory() {
    // TODO: these should probably return functions instead of objects
    presets_["Blank"] = getBlank;
    presets_["Line"] = getLine;
    presets_["Step"] = getStair;
    presets_["Star"] = getStar;
    presets_["Pentagon"] = getPentagon;
    presets_["Basis"] = getBasis;
    presets_["G"] = getG;
    presets_["Circle"] = getCircle;
    presets_["Ellipse"] = getEllipse;
    presets_["JiriTest"] = getJiriTest;
}


Curve conics::ConicPresetFactory::getPreset(const QString &name) const {
    return presets_.at(name)();
}

std::vector<QString> conics::ConicPresetFactory::getPresetNames() const {
    std::vector<QString> keys;
    keys.reserve(presets_.size());
    for (const auto &[key, value]: presets_) {
        keys.emplace_back(key);
    }
    return keys;
}

conics::ConicPresetFactory::~ConicPresetFactory() = default;
