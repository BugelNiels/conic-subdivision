#ifndef CURVE_RENDERER_H
#define CURVE_RENDERER_H

#include <QOpenGLShaderProgram>

#include "renderer.h"
#include "shadertypes.h"
#include "subdivisioncurve.h"

class CurveRenderer : public Renderer {
 public:
  CurveRenderer() {}
  ~CurveRenderer() override;
  void updateBuffers(SubdivisionCurve& sc, bool closed);
  void draw();

 protected:
  void initShaders() override;
  void initBuffers() override;

 private:
  GLuint vao, vbo_coords, vbo_norms, ibo;
  int vboSize;
};

#endif  // CURVE_RENDERER_H
