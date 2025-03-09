#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "conis/core/curve/curve.hpp"
#include "renderer.hpp"
#include "shadertypes.hpp"

namespace conis::gui {

class CurveRenderer final : public Renderer {
public:
    explicit CurveRenderer(const ViewSettings &settings);

    ~CurveRenderer() override;

    void updateBuffers(const core::Curve &curve);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
#if SHADER_DOUBLE_PRECISION
    int numBuffers_ = 4;
#else
    int numBuffers_ = 3;
#endif

    GLuint vao_, ibo_;
    std::vector<GLuint> vbo_;
    QOpenGLTexture *texture_;
    int vboSize_ = 0;
};

} // namespace conis::gui
