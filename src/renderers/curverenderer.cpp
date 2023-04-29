#include "curverenderer.h"

CurveRenderer::~CurveRenderer() {
  gl->glDeleteVertexArrays(1, &vao);
  gl->glDeleteBuffers(1, &vbo_coords);
  gl->glDeleteBuffers(1, &vbo_norms);
  gl->glDeleteBuffers(1, &ibo);
}

void CurveRenderer::initShaders() {
  shaders.insert(ShaderType::POLYLINE, constructPolyLineShader());
}

void CurveRenderer::initBuffers() {
  // Pure OpenGL functions used here

  // create vao
  gl->glGenVertexArrays(1, &vao);
  // bind vao
  gl->glBindVertexArray(vao);

  gl->glGenBuffers(1, &vbo_coords);
  gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
  gl->glEnableVertexAttribArray(0);
  gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  gl->glGenBuffers(1, &vbo_norms);
  gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms);
  gl->glEnableVertexAttribArray(1);
  gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  gl->glGenBuffers(1, &ibo);
  gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

  // unbind
  gl->glBindVertexArray(0);
}

void CurveRenderer::updateBuffers(SubdivisionCurve& sc, bool closed) {
  QVector<QVector2D> coords;
  QVector<QVector2D> normals;
  if (sc.getSubdivLevel() == 0) {
    coords = sc.getNetCoords();
    normals = sc.getNetNormals();
  } else {
    coords = sc.getCurveCoords();
    normals = sc.getCurveNormals();
  }

  gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
  gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords.size(),
                   coords.data(), GL_DYNAMIC_DRAW);

  gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms);
  gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals.size(),
                   normals.data(), GL_DYNAMIC_DRAW);

  QVector<int> indices(coords.size() + 2);
  for (int i = 0; i < coords.size(); i++) {
    indices.append(i);
  }
  if (closed) {
    indices.prepend(coords.size() - 1);
    indices.append(0);
  } else {
    indices.prepend(0);
    indices.append(coords.size() - 1);
  }

  gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                   indices.data(), GL_DYNAMIC_DRAW);

  vboSize = indices.size();
}

void CurveRenderer::draw() {
  // Always renders the control net using the flat shader.
  auto shader = shaders[ShaderType::POLYLINE];
  shader->bind();
  shader->setUniformValue(shader->uniformLocation("visualize_normals"),
                          settings->visualizeNormals);

  gl->glBindVertexArray(vao);

  gl->glDrawElements(GL_LINE_STRIP_ADJACENCY, vboSize, GL_UNSIGNED_INT,
                     nullptr);
  gl->glBindVertexArray(0);

  shader->release();
}
