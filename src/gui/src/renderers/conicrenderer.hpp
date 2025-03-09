#pragma once

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "renderer.hpp"
#include "shadertypes.hpp"

namespace conis::gui {

class ConicRenderer final : public Renderer {
public:
    explicit ConicRenderer(const ViewSettings &settings);

    ~ConicRenderer() override;

    void updateBuffers(const core::Matrix3DD &q);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
    QMatrix3x3 conicCoefs_;
    float conicWidth_ = 0.1;
#if SHADER_DOUBLE_PRECISION
    int numBuffers_ = 4;
#else
    int numBuffers_ = 3;
#endif

    GLuint vao_, ibo_;
    GLuint vbo_;
    int vboSize_ = 0;
};

} // namespace conis::gui
