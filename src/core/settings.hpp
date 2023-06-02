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
    bool visualizeCurvature = false;
    bool visualizeStability = false;
    bool showControlPoints = true;
    bool showControlCurve = true;
    bool normalHandles = true;

    // Calculation Weights
    double pointWeight = 10000.0;
    double normalWeight = 10000.0;
    double middlePointWeight = 1.0;
    double middleNormalWeight = 1.0;

    // Calculation settings
    bool normalizedSolve = false;
    bool circleNormals = false;
    bool recalculateNormals = false;
    bool edgeTangentSample = true;
    bool convexitySplit = true;
    bool areaWeightedNormals = true;
    bool weightedKnotLocation = false;
    bool gravitateSmallerAngles = true;
    double knotTension = 0.8;
    double epsilon = 1e-20;

    // UI related constants
    float normalLength = 0.15;
    float initialScale = 400; // Used to enlarge the curve coordinates to fit in screen coordinates
    float selectRadius = 0.05;
    float deselectRadius = 0.05;
    float drawPointRadius = 8.0f;
    float selectedPointRadius = 12.0f;
    float curveLineWidth = 2.0f;
    float controlLineWidth = 1.0f;
    int curvatureSign = 1;


    // UI Appearance
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
    conics::ui::StylePreset style;

    // UI Controls
    float zoomStrength = 1.2;
    float dragSensitivity = 1;


} Settings;
