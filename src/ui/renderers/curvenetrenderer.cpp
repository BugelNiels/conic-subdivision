#include "curvenetrenderer.hpp"

#include "core/settings.hpp"

CurveNetRenderer::CurveNetRenderer(const Settings &settings) : Renderer(settings) {

}

/**
 * @brief CurveNetRenderer::~CurveNetRenderer Destructor of the control curve
 * net renderer.
 */
CurveNetRenderer::~CurveNetRenderer() {
    gl_->glDeleteVertexArrays(1, &vao_);
    gl_->glDeleteBuffers(1, &vbo_coords_);
    gl_->glDeleteBuffers(1, &vbo_norms_);
    gl_->glDeleteBuffers(1, &ibo_);
}

/**
 * @brief CurveNetRenderer::initShaders Initialises the different shaders used
 * in the control curve net renderer. The control net only supports
 * the flat shader.
 */
void CurveNetRenderer::initShaders() {
    shaders_.insert(ShaderType::FLAT, constructDefaultShader("flat"));
}

/**
 * @brief CurveNetRenderer::initBuffers Initialises the buffers
 */
void CurveNetRenderer::initBuffers() {
    // Pure OpenGL functions used here

    // create vao
    gl_->glGenVertexArrays(1, &vao_);
    // bind vao
    gl_->glBindVertexArray(vao_);

    gl_->glGenBuffers(1, &vbo_coords_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl_->glEnableVertexAttribArray(0);
    gl_->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glGenBuffers(1, &ibo_);
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    // unbind
    gl_->glBindVertexArray(0);
}

/**
 * @brief CurveNetRenderer::updateBuffers Updates the contents of the buffers.
 * @param sc The subdivision curve containing the information to update the
 * buffer(s) with.
 */
void CurveNetRenderer::updateBuffers(SubdivisionCurve &sc) {
    coords_ = sc.getNetCoords();
    normals_ = sc.getNetNormals();
    if (coords_.size() == 0) {
        vboSize_ = 0;
        return;
    }
    int size = coords_.size();
    for (int i = 0; i < size; i++) {
        normals_[i] = (coords_[i] + settings_.normalLength * normals_[i]);
    }

    QVector<int> indices(coords_.size() + 2);
    for (int i = 0; i < coords_.size(); i++) {
        indices.append(i);
    }
    if (sc.isClosed()) {
        indices.prepend(coords_.size() - 1);
        indices.append(0);
    } else {
        indices.prepend(0);
        indices.append(coords_.size() - 1);
    }

    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                      indices.data(), GL_DYNAMIC_DRAW);

    vboSize_ = indices.size();
}

/**
 * @brief CurveNetRenderer::draw Draw call. Draws the control net on the screen.
 * Uses the contents of the vertex array buffer for drawing. The size of the
 * draw array is expected to be stored in vboSize.
 */
void CurveNetRenderer::draw() {
    // Always renders the control net using the flat shader.
    auto shader = shaders_[ShaderType::FLAT];
    shader->bind();

    if (vboSize_ == 0) {
        return;
    }

    gl_->glLineWidth(settings_.controlLineWidth);
    gl_->glBindVertexArray(vao_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords_.size(),
                      coords_.data(), GL_DYNAMIC_DRAW);

    shader->setUniformValue("projectionMatrix", settings_.projectionMatrix);
    shader->setUniformValue(shader->uniformLocation("viewMatrix"),
                            settings_.viewMatrix);

    // Control Curve
    if (settings_.showControlCurve) {
        QColor qCol = settings_.style.controlCurveCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl_->glDrawElements(GL_LINE_STRIP, vboSize_, GL_UNSIGNED_INT, nullptr);
    }
    // Control points
    if (settings_.showControlPoints) {
        QColor qCol = settings_.style.controlPointCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());

        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl_->glPointSize(settings_.drawPointRadius);
        gl_->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
    }

    // Highlight selected control point
    if (settings_.selectedVertex > -1) {
        QColor qCol = settings_.style.selectedVertCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl_->glPointSize(settings_.selectedPointRadius);
        gl_->glDrawArrays(GL_POINTS, settings_.selectedVertex, 1);
    }

    if (settings_.normalHandles) {
        gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
        gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals_.size(),
                          normals_.data(), GL_DYNAMIC_DRAW);
        QColor qCol = settings_.style.normCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl_->glPointSize(settings_.drawPointRadius);
        gl_->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
        if (settings_.selectedNormal > -1) {
            gl_->glPointSize(settings_.selectedPointRadius);
            QColor secQCol = settings_.style.selectedNormCol;
            QVector3D secCol(secQCol.redF(), secQCol.greenF(), secQCol.blueF());
            shader->setUniformValue(shader->uniformLocation("lineColor"), secCol);
            gl_->glDrawArrays(GL_POINTS, settings_.selectedNormal, 1);
        }
    }

    shader->release();
}
