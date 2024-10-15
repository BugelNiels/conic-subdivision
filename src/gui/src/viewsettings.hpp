#pragma once

#include <QMatrix4x4>

#include "conis/core/curve/curvaturetype.hpp"
#include "renderers/shadertypes.hpp"
#include "stylepresets.hpp"

namespace conis::gui {

using ViewSettings = struct ViewSettings {
    // Selection data
    int highlightedVertex = -1;
    int highlightedNormal = -1;
    int selectedVertex = -1;
    int highlightedEdge = -1;
    int selectedEdge = -1;

    // View Settings
    bool visualizeNormals = false;
    bool visualizeCurvature = false;
    bool showControlPoints = true;
    bool showControlCurve = true;
    bool normalHandles = true;
    bool drawSelectedConic = true;
    bool constrainNormalMovement = true;
    conis::core::CurvatureType curvatureType = conis::core::CurvatureType::CIRCLE_RADIUS;


    // UI related constants
    float normalLength = 0.15;
    float initialScale = 400; // Used to enlarge the curve coordinates to fit in screen coordinates
    float selectRadius = 0.05;
    float deselectRadius = 0.05;
    float drawPointRadius = 8.0f;
    
    // Draw settings
    float conicWidth = 0.01f;
    float curveLineWidth = 3.0f;
    float controlLineWidth = 2.0f;
    float selectedLineWidth = 3.0f;
    float highlightedLineWidth = 2.0f;
    float selectedPointRadius = 15.0f;
    float highlightedPointRadius = 12.0f;
    float curvatureScale = 1.0f;
    int curvatureSign = 1;

    // UI Appearance
    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;
    StylePreset style;

    // UI Controls
    float zoomStrength = 1.2;
    float dragSensitivity = 1;
};

} // namespace conis::gui