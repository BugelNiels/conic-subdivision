#pragma once

#include <QOpenGLShaderProgram>

#include "renderer.hpp"
#include "shadertypes.hpp"
#include "src/core/subdivisioncurve.hpp"

/**
 * @brief Renderer responsible for rendering the control net.
 */
class CurveNetRenderer : public Renderer {
public:
    explicit CurveNetRenderer(const Settings &settings);

    ~CurveNetRenderer() override;

    void updateBuffers(SubdivisionCurve &sc);

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
