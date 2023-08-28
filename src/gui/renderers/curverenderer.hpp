#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "renderer.hpp"
#include "shadertypes.hpp"
#include "src/core/subdivisioncurve.hpp"

class CurveRenderer : public Renderer {
public:
    explicit CurveRenderer(const Settings &settings);

    ~CurveRenderer() override;

    void updateBuffers(SubdivisionCurve &sc);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
#ifdef SHADER_DOUBLE_PRECISION
    int numBuffers_ = 4;
#else
    int numBuffers_ = 3;
#endif

    GLuint vao_, ibo_;
    std::vector<GLuint> vbo_;
    QOpenGLTexture *texture_;
    int vboSize_ = 0;
};