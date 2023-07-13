#include "conicpresets.hpp"

#include "util/vector.hpp"

conics::ConicPresets::ConicPresets(const Settings &settings) : settings_(settings) {

    presets_["Blank"] = std::make_shared<SubdivisionCurve>(getBlank());
    presets_["Line"] = std::make_shared<SubdivisionCurve>(getLine());
    presets_["Step"] = std::make_shared<SubdivisionCurve>(getStair());
    presets_["Star"] = std::make_shared<SubdivisionCurve>(getStar());
    presets_["Pentagon"] = std::make_shared<SubdivisionCurve>(getPentagon());
    presets_["Basis"] = std::make_shared<SubdivisionCurve>(getBasis());
    presets_["G"] = std::make_shared<SubdivisionCurve>(getG());
    presets_["Circle"] = std::make_shared<SubdivisionCurve>(getCircle(4, 0.5));
    presets_["Ellipse"] = std::make_shared<SubdivisionCurve>(getEllipse(5, 0.8, 0.3));
}


conics::ConicPresets::~ConicPresets() = default;

SubdivisionCurve conics::ConicPresets::getBlank() {
    std::vector<Vector2DD> netCoords;
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getLine() {
    std::vector<Vector2DD> netCoords;
    netCoords.emplace_back(-0.75, 0);
    netCoords.emplace_back(0.75, 0);
    return SubdivisionCurve(settings_, netCoords, false);
}

SubdivisionCurve conics::ConicPresets::getStair() {
    std::vector<Vector2DD> netCoords;
    netCoords.emplace_back(-1, 1);
    netCoords.emplace_back(1, 1);
    netCoords.emplace_back(1, -1);
    netCoords.emplace_back(0.3, -1);
    netCoords.emplace_back(0.3, -.3);
    netCoords.emplace_back(-0.3, -.3);
    netCoords.emplace_back(-0.3, .3);
    netCoords.emplace_back(-1, .3);
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getStar() {
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
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getPentagon() {
    std::vector<Vector2DD> netCoords;
    netCoords.reserve(5);
    netCoords.emplace_back(-0.25, -0.5);
    netCoords.emplace_back(-0.75, 0.0);
    netCoords.emplace_back(-0.25, 0.75);
    netCoords.emplace_back(0.75, 0.5);
    netCoords.emplace_back(0.5, -0.75);
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getBasis() {
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
    return SubdivisionCurve(settings_, netCoords, false);
}

SubdivisionCurve conics::ConicPresets::getG() {
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
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getCircle(int numPoints, double radius) {
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
    return {settings_, netCoords, netNormals};
}

SubdivisionCurve conics::ConicPresets::getEllipse(int numPoints, double width, double height) {
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
    return {settings_, netCoords, netNormals};
}


SubdivisionCurve conics::ConicPresets::getPreset(const QString &name) const {
    return *presets_.at(name);
}

std::vector<QString> conics::ConicPresets::getPresetNames() const {
    std::vector<QString> keys;
    keys.reserve(presets_.size());
    for (const auto& [key, value]: presets_) {
        keys.emplace_back(key);
    }
    return keys;
}
