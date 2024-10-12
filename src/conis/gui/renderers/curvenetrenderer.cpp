#include "curvenetrenderer.hpp"

namespace conis::gui {

CurveNetRenderer::CurveNetRenderer(const ViewSettings &settings) : Renderer(settings) {}

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
    shaders_.insert(ShaderType::DASHED, constructDefaultShader("dashed"));
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
 * @param curve The curve containing the information to update the
 * buffer(s) with.
 */
void CurveNetRenderer::updateBuffers(const conis::core::Curve &curve) {
    coords_ = qVecToVec(curve.getCoords());
    normals_ = qVecToVec(curve.getNormals());
    int numVerts = coords_.size();
    if (numVerts == 0) {
        vboSize_ = 0;
        return;
    }
    for (int i = 0; i < numVerts; i++) {
        normals_[i] = (coords_[i] + settings_.normalLength * normals_[i].normalized());
    }

    std::vector<int> indices;
    indices.reserve(numVerts + 2);
    indices.emplace_back(curve.isClosed() ? numVerts - 1 : 0);
    for (int i = 0; i < numVerts; i++) {
        indices.emplace_back(i);
    }
    if (curve.isClosed()) {
        indices.emplace_back(0);
        // Technically this shouldn't be necessary with indexed rendering
        // However, the line segment drawing is not based on the index buffer
        // so we need this to be able to draw a line segment from the last vertex to the first
        coords_.push_back(coords_[0]);
        normals_.push_back(normals_[0]);
    } else {
        indices.emplace_back(numVerts - 1);
    }
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

    vboSize_ = indices.size();
}

void CurveNetRenderer::drawPoint(QOpenGLShaderProgram *shader, int idx, int radius, QColor qCol) {
    if (idx < 0 || idx >= vboSize_) {
        return;
    }
    QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
    shader->setUniformValue("lineColor", col);
    gl_->glPointSize(radius);
    gl_->glDrawArrays(GL_POINTS, idx, 1);
}

void CurveNetRenderer::drawLine(QOpenGLShaderProgram *shader, int pIdx, QColor qCol, float lineWidth) {
    if (pIdx < 0 || pIdx >= vboSize_) {
        return; // Ensure indices are valid
    }

    // Set line color
    QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
    shader->setUniformValue("lineColor", col);

    // Set line width
    gl_->glLineWidth(lineWidth);
    gl_->glDrawArrays(GL_LINES, pIdx, 2);
}

/**
 * @brief CurveNetRenderer::draw Draw call. Draws the control net on the screen.
 * Uses the contents of the vertex array buffer for drawing. The size of the
 * draw array is expected to be stored in vboSize.
 */
void CurveNetRenderer::draw() {
    if (vboSize_ == 0) {
        return;
    }
    auto shader = shaders_[ShaderType::DASHED];
    shader->bind();

    gl_->glLineWidth(settings_.controlLineWidth);
    gl_->glBindVertexArray(vao_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords_.size(), coords_.data(), GL_DYNAMIC_DRAW);

    shader->setUniformValue("projectionMatrix", settings_.projectionMatrix);
    shader->setUniformValue("viewMatrix", settings_.viewMatrix);
    shader->setUniformValue("dashEnabled", true);

    // Control Curve
    if (settings_.showControlCurve) {
        QColor qCol = settings_.style.controlCurveCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue("lineColor", col);
        gl_->glDrawElements(GL_LINE_STRIP, vboSize_, GL_UNSIGNED_INT, nullptr);
    }

    shader->setUniformValue("dashEnabled", false);
    // Selected curve segment (if any)
    drawLine(shader, settings_.highlightedEdge, settings_.style.controlCurveCol, settings_.highlightedLineWidth);
    drawLine(shader, settings_.selectedEdge, settings_.style.controlCurveCol, settings_.selectedLineWidth);

    // Control points
    if (settings_.showControlPoints) {
        QColor qCol = settings_.style.controlPointCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());

        shader->setUniformValue("lineColor", col);
        gl_->glPointSize(settings_.drawPointRadius);
        gl_->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
    }

    // Selected Control Point (if any)
    drawPoint(shader,
              settings_.highlightedVertex,
              settings_.highlightedPointRadius,
              settings_.style.highlightedVertCol);
    drawPoint(shader, settings_.selectedVertex, settings_.selectedPointRadius, settings_.style.selectedVertCol);

    // Normal handles
    if (settings_.normalHandles) {
        gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
        gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals_.size(), normals_.data(), GL_DYNAMIC_DRAW);
        QColor qCol = settings_.style.normCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue("lineColor", col);
        gl_->glPointSize(settings_.drawPointRadius);
        gl_->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
        // Highlighted normal handle
        drawPoint(shader,
                  settings_.highlightedNormal,
                  settings_.highlightedPointRadius,
                  settings_.style.selectedNormCol);
    }

    shader->release();
}

} // namespace conis::gui
