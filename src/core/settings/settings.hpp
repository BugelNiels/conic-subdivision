#pragma once

#include <QMatrix4x4>

#include "gui/renderers/shadertypes.hpp"
#include "gui/stylepresets.hpp"
#include "core/vector.hpp"

// TODO: move this outside of code (and split)
namespace conics::core {

/**
 * Struct that contains all the settings of the program. Initialised with a
 * number of default values.
 */
using Settings = struct Settings {
    // Selection data
    // TODO: move into controller
    int highlightedVertex = -1;
    int highlightedNormal = -1;
    int selectedVertex = -1;

    // View Settings
    bool visualizeNormals = false;
    bool visualizeCurvature = false;
    bool showControlPoints = true;
    bool showControlCurve = true;
    bool normalHandles = true;

    // Calculation Weights
    real_t middlePointWeight = 100000.0;
    real_t middleNormalWeight = 100000.0;
    // The ratio between this and outerNormalWeight is Tau in the paper
    real_t outerPointWeight = 100.0;
    real_t outerNormalWeight = 1.0;
    int patchSize = 2;

    // Normal refinement
    int maxRefinementIterations = 1;
    real_t angleLimit = 1.0e-8;
    int testSubdivLevel = 6;

    // Calculation settings
    bool circleNormals = false;
    bool convexitySplit = true;
    bool areaWeightedNormals = true;
    bool weightedInflPointLocation = false;
    bool gravitateSmallerAngles = true;
    real_t epsilon = 1e-40;
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
};

} // namespace conics::core