#pragma once

#include "gui/stylepresets.hpp"
#include "src/gui/renderers/shadertypes.hpp"
#include <QMatrix4x4>

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
    bool visualizeCurvature = false;
    bool showControlPoints = true;
    bool showControlCurve = true;
    bool normalHandles = true;

    // Calculation Weights
    double middlePointWeight = 100000.0;
    double middleNormalWeight = 100000.0;
    double outerPointWeight = 10.0;
    double outerNormalWeight = 1.0;
    int patchSize = 2;

    // Calculation settings
    bool circleNormals = false;
    bool convexitySplit = true;
    bool areaWeightedNormals = true;
    bool weightedInflPointLocation = false;
    bool gravitateSmallerAngles = true;
    long double epsilon = 1e-40;
    bool dynamicPatchSize = true;

    // UI related constants
    float normalLength = 0.15;
    float initialScale = 400; // Used to enlarge the curve coordinates to fit in screen coordinates
    float selectRadius = 0.05;
    float deselectRadius = 0.05;
    float drawPointRadius = 8.0f;
    float selectedPointRadius = 12.0f;
    float curveLineWidth = 3.0f;
    float controlLineWidth = 2.0f;
    int curvatureSign = 1;
    float curvatureScale = 1.0f;

    // For testing purposes
    bool testToggle = false;

    // UI Appearance
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
    conics::ui::StylePreset style;

    // UI Controls
    float zoomStrength = 1.2;
    float dragSensitivity = 1;
} Settings;
