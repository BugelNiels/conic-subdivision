#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "renderer.hpp"
#include "shadertypes.hpp"
#include "src/core/subdivisioncurve.hpp"

class CurveRenderer : public Renderer {
public:
    CurveRenderer(Settings *settings);

    ~CurveRenderer() override;

    void updateBuffers(SubdivisionCurve &sc);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
    GLuint vao_, vbo_coords_, vbo_norms_, vbo_stab_, ibo_;
    QOpenGLTexture *texture_;
    int vboSize_ = 0;
};
