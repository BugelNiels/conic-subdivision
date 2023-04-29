#ifndef SETTINGS_H
#define SETTINGS_H

#include "shadertypes.h"

/**
 * Struct that contains all the settings of the program. Initialised with a
 * number of default values.
 */
typedef struct Settings {
  bool showNet = true;
  bool showSubdivisionCurve = false;
  int selectedPoint = -1;

  ShaderType currentShader = ShaderType::FLAT;

  bool normalizedSolve = true;
  bool circleNormals = false;
  bool recalculateNormals = true;

  bool visualizeNormals = false;
  bool outwardNormals = false;
  bool edgeTangentSample = true;
  bool closed = true;

  double pointWeight = 100.0;
  double normalWeight = 100.0;
  double middlePointWeight = 2.0;
  double middleNormalWeight = 2.0;
  double outerPointWeight = 1.0;
  double outerNormalWeight = 1.0;

  bool showControlPoints = true;
  bool showControlCurve = true;

} Settings;

#endif  // SETTINGS_H
