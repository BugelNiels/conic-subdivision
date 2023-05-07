#include "conicpresets.hpp"

conics::ConicPresets::ConicPresets(Settings *settings) : settings_(settings) {

    presets_["Pentagon"] = getPentagon();
    presets_["Basis"] = getBasis();
    presets_["G"] = getG();
    presets_["Circle"] = getCircle(5, 0.5f);
    presets_["Ellipse"] = getEllipse(5, 0.8f, 0.3f);
}


conics::ConicPresets::~ConicPresets() {

}


SubdivisionCurve conics::ConicPresets::getPentagon() {
    QVector<QVector2D> netCoords;
    netCoords.reserve(5);
    netCoords.append(QVector2D(-0.25f, -0.5f));
    netCoords.append(QVector2D(-0.75f, 0.0f));
    netCoords.append(QVector2D(-0.25f, 0.75f));
    netCoords.append(QVector2D(0.75f, 0.5f));
    netCoords.append(QVector2D(0.5f, -0.75f));
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getBasis() {
    QVector<QVector2D> netCoords;
    netCoords.reserve(9);
    netCoords.append(QVector2D(-1.0f, -0.25f));
    netCoords.append(QVector2D(-0.75f, -0.25f));
    netCoords.append(QVector2D(-0.5f, -0.25f));
    netCoords.append(QVector2D(-0.25f, -0.25f));
    netCoords.append(QVector2D(0.0f, 0.50f));
    netCoords.append(QVector2D(0.25f, -0.25f));
    netCoords.append(QVector2D(0.5f, -0.25f));
    netCoords.append(QVector2D(0.75f, -0.25f));
    netCoords.append(QVector2D(1.0f, -0.25f));
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getG() {
    QVector<QVector2D> netCoords;
    netCoords.reserve(14);
    netCoords.append(QVector2D(0.75f, 0.35f));
    netCoords.append(QVector2D(0.75f, 0.75f));
    netCoords.append(QVector2D(-0.75f, 0.75f));
    netCoords.append(QVector2D(-0.75f, -0.75f));
    netCoords.append(QVector2D(0.75f, -0.75f));
    netCoords.append(QVector2D(0.75f, 0.0f));
    netCoords.append(QVector2D(0.0f, 0.0f));
    netCoords.append(QVector2D(0.0f, -0.2f));
    netCoords.append(QVector2D(0.55f, -0.2f));
    netCoords.append(QVector2D(0.55f, -0.55f));
    netCoords.append(QVector2D(-0.55f, -0.55f));
    netCoords.append(QVector2D(-0.55f, 0.55f));
    netCoords.append(QVector2D(0.55f, 0.55f));
    netCoords.append(QVector2D(0.55f, 0.35f));
    return SubdivisionCurve(settings_, netCoords);
}

SubdivisionCurve conics::ConicPresets::getCircle(int numPoints, float radius) {
    QVector<QVector2D> netCoords;
    QVector<QVector2D> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const float theta = i * (2.0f * M_PI / numPoints);
        const float x = radius * std::cos(theta);
        const float y = radius * std::sin(theta);
        netCoords.append(QVector2D(x, y));
        netNormals.append(QVector2D(x, y).normalized());
    }
    return SubdivisionCurve(settings_, netCoords, netNormals);
}

SubdivisionCurve conics::ConicPresets::getEllipse(int numPoints, float width, float height) {
    QVector<QVector2D> netCoords;
    QVector<QVector2D> netNormals;
    netCoords.reserve(numPoints);

    for (int i = 0; i < numPoints; ++i) {
        const float theta = i * (2.0f * M_PI / numPoints);
        const float x = width * std::cos(theta);
        const float y = height * std::sin(theta);
        netCoords.append(QVector2D(x, y));
        const float tx = -width * std::sin(theta);
        const float ty = height * std::cos(theta);
        const QVector2D normal(ty, -tx);
        netNormals.append(normal.normalized());
    }
    return SubdivisionCurve(settings_, netCoords, netNormals);
}


SubdivisionCurve conics::ConicPresets::getPreset(const QString &name) const {
    return presets_[name];
}

QList<QString> conics::ConicPresets::getPresetNames() const {
    return presets_.keys();
}
