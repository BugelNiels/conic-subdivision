#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "core/curve/curve.hpp"
#include "renderer.hpp"
#include "shadertypes.hpp"

namespace conics::ui {

class CurveRenderer : public Renderer {
public:
    explicit CurveRenderer(const conics::core::Settings &settings);

    ~CurveRenderer() override;

    void updateBuffers(const conics::core::Curve &curve);

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

} // namespace conics::ui