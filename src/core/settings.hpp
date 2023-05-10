#pragma once

#include "src/ui/renderers/shadertypes.hpp"
#include <QMatrix4x4>
#include "ui/stylepresets.hpp"

/**
 * Struct that contains all the settings of the program. Initialised with a
 * number of default values.
 */
typedef struct Settings {
    // Selection data
    int selectedVertex = -1;
    int selectedNormal = -1;

    // View Settings
    bool visualizeNormals = false;
    bool outwardNormals = false;
    bool showControlPoints = true;
    bool showControlCurve = true;
    bool normalHandles = true;

    // Calculation Weights
    double pointWeight = 1000.0;
    double normalWeight = 100.0;
    double middlePointWeight = 2.0;
    double middleNormalWeight = 2.0;
    double outerPointWeight = 0.0;
    double outerNormalWeight = 0.0;

    // Calculation settings
    bool normalizedSolve = false;
    bool circleNormals = false;
    bool recalculateNormals = false;
    bool edgeTangentSample = true;
    bool convexitySplit = true;
    bool areaWeightedKnot = false;
    bool tessellate = true;

    // UI related constants
    float normalLength = 0.15;
    float sizeCorrection = 500; // Used to enlarge the curve coordinates to fit in screen coordinates
    float selectRadius = 0.05;
    float deselectRadius = 0.05;
    float drawPointRadius = 8.0f;
    float selectedPointRadius = 12.0f;
    float curveLineWidth = 2.0f;
    float controlLineWidth = 1.0f;

    // UI Appearance
    QMatrix4x4 projectionMatrix;
    conics::ui::StylePreset style;

} Settings;
