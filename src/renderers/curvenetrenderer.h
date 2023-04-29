#ifndef CURVE_NET_RENDERER_H
#define CURVE_NET_RENDERER_H

#include <QOpenGLShaderProgram>

#include "renderer.h"
#include "shadertypes.h"
#include "subdivisioncurve.h"

/**
 * @brief Renderer responsible for rendering the control net.
 */
class CurveNetRenderer : public Renderer {
 public:
  CurveNetRenderer() {}
  ~CurveNetRenderer() override;
  void updateBuffers(SubdivisionCurve& sc, bool closed);
  void draw();

 protected:
  void initShaders() override;
  void initBuffers() override;

 private:
  GLuint vao, vbo_coords, vbo_norms, ibo;
  int vboSize;
};

#endif  // CURVE_NET_RENDERER_H
