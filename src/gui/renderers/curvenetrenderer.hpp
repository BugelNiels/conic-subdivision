#pragma once

#include <QOpenGLShaderProgram>

#include "core/curve/curve.hpp"
#include "renderer.hpp"
#include "shadertypes.hpp"

namespace conics::ui {

/**
 * @brief Renderer responsible for rendering the control net.
 */
class CurveNetRenderer : public Renderer {
public:
    explicit CurveNetRenderer(const conics::core::Settings &settings);

    ~CurveNetRenderer() override;

    void updateBuffers(const conics::core::Curve &curve);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

private:
    GLuint vao_, vbo_coords_, vbo_norms_, ibo_;
    int vboSize_ = 0;
    std::vector<QVector2D> coords_;
    std::vector<QVector2D> normals_;
};

} // namespace conics::ui