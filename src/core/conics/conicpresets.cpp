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
    presets_["Circle"] = std::make_shared<SubdivisionCurve>(getCircle(5, 0.5f));
    presets_["Ellipse"] = std::make_shared<SubdivisionCurve>(getEllipse(5, 0.8f, 0.3f));
}


conics::ConicPresets::~ConicPresets() = default;

SubdivisionCurve conics::ConicPresets::getBlank() {
    QVector<Vector2DD> netCoords;
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getLine() {
    QVector<Vector2DD> netCoords;
    netCoords.append(Vector2DD(-0.75f, 0));
    netCoords.append(Vector2DD(0.75f, 0));
    return SubdivisionCurve(settings_, netCoords, false);
}

SubdivisionCurve conics::ConicPresets::getStair() {
    QVector<Vector2DD> netCoords;
    netCoords.append(Vector2DD(-1, 1));
    netCoords.append(Vector2DD(1, 1));
    netCoords.append(Vector2DD(1, -1));
    netCoords.append(Vector2DD(0.3, -1));
    netCoords.append(Vector2DD(0.3, -.3));
    netCoords.append(Vector2DD(-0.3, -.3));
    netCoords.append(Vector2DD(-0.3, .3));
    netCoords.append(Vector2DD(-1, .3));
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getStar() {
    QVector<Vector2DD> netCoords;
    int numPoints = 5;
    netCoords.reserve(numPoints * 2);
    float radius = 1;

    for (int i = 0; i < numPoints; ++i) {
        // Outer points
        float theta = i * (2.0f * M_PI / numPoints);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);
        netCoords.append(Vector2DD(x, y));

        // Inner points
        theta += M_PI / numPoints;
        x = 0.3f * radius * std::cos(theta);
        y = 0.3f * radius * std::sin(theta);
        netCoords.append(Vector2DD(x, y));
    }
    return SubdivisionCurve(settings_, netCoords, true);
}

SubdivisionCurve conics::ConicPresets::getPentagon() {
    QVector<Vector2DD> netCoords;
    netCoords.reserve(5);
    netCoords.append(Vector2DD(-0.25f, -0.5f));
    netCoords.append(Vector2DD(-0.75f, 0.0f));
    netCoords.append(Vector2DD(-0.25f, 0.75f));
    netCoords.append(Vector2DD(0.75f, 0.5f));
    netCoords.append(Vector2DD(0.5f, -0.75f));
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getBasis() {
    QVector<Vector2DD> netCoords;
    netCoords.reserve(9);
    netCoords.append(Vector2DD(-1.0f, -0.25f));
    netCoords.append(Vector2DD(-0.75f, -0.25f));
    netCoords.append(Vector2DD(-0.5f, -0.25f));
    netCoords.append(Vector2DD(-0.25f, -0.25f));
    netCoords.append(Vector2DD(0.0f, 0.50f));
    netCoords.append(Vector2DD(0.25f, -0.25f));
    netCoords.append(Vector2DD(0.5f, -0.25f));
    netCoords.append(Vector2DD(0.75f, -0.25f));
    netCoords.append(Vector2DD(1.0f, -0.25f));
    return SubdivisionCurve(settings_, netCoords, false);
}

SubdivisionCurve conics::ConicPresets::getG() {
    QVector<Vector2DD> netCoords;
    netCoords.reserve(14);
    netCoords.append(Vector2DD(0.75f, 0.35f));
    netCoords.append(Vector2DD(0.75f, 0.75f));
    netCoords.append(Vector2DD(-0.75f, 0.75f));
    netCoords.append(Vector2DD(-0.75f, -0.75f));
    netCoords.append(Vector2DD(0.75f, -0.75f));
    netCoords.append(Vector2DD(0.75f, 0.0f));
    netCoords.append(Vector2DD(0.0f, 0.0f));
    netCoords.append(Vector2DD(0.0f, -0.2f));
    netCoords.append(Vector2DD(0.55f, -0.2f));
    netCoords.append(Vector2DD(0.55f, -0.55f));
    netCoords.append(Vector2DD(-0.55f, -0.55f));
    netCoords.append(Vector2DD(-0.55f, 0.55f));
    netCoords.append(Vector2DD(0.55f, 0.55f));
    netCoords.append(Vector2DD(0.55f, 0.35f));
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getCircle(int numPoints, float radius) {
    QVector<Vector2DD> netCoords;
    QVector<Vector2DD> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const float theta = i * (2.0f * M_PI / numPoints);
        const float x = radius * std::cos(theta);
        const float y = radius * std::sin(theta);
        netCoords.append(Vector2DD(x, y));
        netNormals.append(Vector2DD(x, y).normalized());
    }
    return SubdivisionCurve(settings_, netCoords, netNormals);
}

SubdivisionCurve conics::ConicPresets::getEllipse(int numPoints, float width, float height) {
    QVector<Vector2DD> netCoords;
    QVector<Vector2DD> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const float theta = i * (2.0f * M_PI / numPoints);
        const float x = width * std::cos(theta);
        const float y = height * std::sin(theta);
        netCoords.append(Vector2DD(x, y));
        const float tx = -width * std::sin(theta);
        const float ty = height * std::cos(theta);
        const Vector2DD normal(ty, -tx);
        netNormals.append(normal.normalized());
    }
    return SubdivisionCurve(settings_, netCoords, netNormals);
}


SubdivisionCurve conics::ConicPresets::getPreset(const QString &name) const {
    return *presets_[name];
}

QList<QString> conics::ConicPresets::getPresetNames() const {
    return presets_.keys();
}
