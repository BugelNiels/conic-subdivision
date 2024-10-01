#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "renderer.hpp"
#include "shadertypes.hpp"

class ConicRenderer : public Renderer {
public:
    explicit ConicRenderer(const Settings &settings);

    ~ConicRenderer() override;

    void updateBuffers(Matrix3DD &q);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
    QMatrix3x3 conicCoefs_;
#ifdef SHADER_DOUBLE_PRECISION
    int numBuffers_ = 4;
#else
    int numBuffers_ = 3;
#endif

    GLuint vao_, ibo_;
    GLuint vbo_;
    int vboSize_ = 0;
    bool shouldDraw_ = true;
};
