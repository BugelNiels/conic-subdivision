#pragma once

#include <QOpenGLShaderProgram>

#include "conis/core/curve/curve.hpp"
#include "renderer.hpp"
#include "shadertypes.hpp"

namespace conis::gui {

/**
 * @brief Renderer responsible for rendering the control net.
 */
class CurveNetRenderer : public Renderer {
public:
    explicit CurveNetRenderer(const ViewSettings &settings);

    ~CurveNetRenderer() override;

    void updateBuffers(const conis::core::Curve &curve);

    void draw();

protected:
    void initShaders() override;

    void initBuffers() override;

    void drawPoint(QOpenGLShaderProgram *shader, int idx, int radius, QColor qCol);
    void drawLine(QOpenGLShaderProgram *shader, int pIdx, QColor qCol, float lineWidth) ;

private:
    GLuint vao_, vbo_coords_, vbo_norms_, ibo_;
    int vboSize_ = 0;
    std::vector<QVector2D> vertices_;
    std::vector<QVector2D> normals_;
};

} // namespace conis::gui