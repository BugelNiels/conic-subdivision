#pragma once

#include <QOpenGLShaderProgram>

#include "renderer.hpp"
#include "shadertypes.hpp"
#include "src/core/subdivisioncurve.hpp"

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
  int vboSize = 0;
};
