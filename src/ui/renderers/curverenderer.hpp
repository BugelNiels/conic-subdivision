#pragma once

#include <QOpenGLShaderProgram>

#include "renderer.hpp"
#include "shadertypes.hpp"
#include "src/core/subdivisioncurve.hpp"

class CurveRenderer : public Renderer {
 public:
  CurveRenderer() {}
  ~CurveRenderer() override;
  void updateBuffers(SubdivisionCurve& sc);
  void draw();

 protected:
  void initShaders() override;
  void initBuffers() override;

 private:
  GLuint vao_, vbo_coords_, vbo_norms_, ibo_;
  int vboSize_ = 0;
};
