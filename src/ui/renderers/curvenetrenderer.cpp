#include "curvenetrenderer.hpp"

#include "core/settings.hpp"

/**
 * @brief CurveNetRenderer::~CurveNetRenderer Destructor of the control curve
 * net renderer.
 */
CurveNetRenderer::~CurveNetRenderer() {
    gl->glDeleteVertexArrays(1, &vao_);
    gl->glDeleteBuffers(1, &vbo_coords_);
    gl->glDeleteBuffers(1, &vbo_norms_);
    gl->glDeleteBuffers(1, &ibo_);
}

/**
 * @brief CurveNetRenderer::initShaders Initialises the different shaders used
 * in the control curve net renderer. The control net only supports
 * the flat shader.
 */
void CurveNetRenderer::initShaders() {
    shaders.insert(ShaderType::FLAT, constructDefaultShader("flat"));
}

/**
 * @brief CurveNetRenderer::initBuffers Initialises the buffers
 */
void CurveNetRenderer::initBuffers() {
    // Pure OpenGL functions used here

    // create vao
    gl->glGenVertexArrays(1, &vao_);
    // bind vao
    gl->glBindVertexArray(vao_);

    gl->glGenBuffers(1, &vbo_coords_);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);


    gl->glGenBuffers(1, &ibo_);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    // unbind
    gl->glBindVertexArray(0);
}

/**
 * @brief CurveNetRenderer::updateBuffers Updates the contents of the buffers.
 * @param sc The subdivision curve containing the information to update the
 * buffer(s) with.
 */
void CurveNetRenderer::updateBuffers(SubdivisionCurve &sc, bool closed) {
    coords_ = sc.getNetCoords();
    normals_ = sc.getNetNormals();
    if (coords_.size() == 0) {
        return;
    }
    int size = coords_.size();
    for (int i = 0; i < size; i++) {
        normals_[i] = (coords_.at(i) + settings->normalLength * normals_.at(i));
    }

    QVector<int> indices(coords_.size() + 2);
    for (int i = 0; i < coords_.size(); i++) {
        indices.append(i);
    }
    if (closed) {
        indices.prepend(coords_.size() - 1);
        indices.append(0);
    } else {
        indices.prepend(0);
        indices.append(coords_.size() - 1);
    }

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
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
    auto shader = shaders[ShaderType::FLAT];
    shader->bind();

    gl->glLineWidth(settings->controlLineWidth);
    gl->glBindVertexArray(vao_);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords_.size(),
                     coords_.data(), GL_DYNAMIC_DRAW);

    shader->setUniformValue("projectionMatrix", settings->projectionMatrix);

    // Control Curve
    if (settings->showControlCurve) {
        QColor qCol = settings->style.controlCurveCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl->glDrawElements(GL_LINE_STRIP, vboSize_, GL_UNSIGNED_INT, nullptr);
    }
    // Control points
    if (settings->showControlPoints) {
        QColor qCol = settings->style.controlPointCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());

        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl->glPointSize(settings->drawPointRadius);
        gl->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
    }

    // Highlight selected control point
    if (settings->selectedVertex > -1) {
        QColor qCol = settings->style.selectedVertCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl->glPointSize(settings->selectedPointRadius);
        gl->glDrawArrays(GL_POINTS, settings->selectedVertex, 1);
    }

    if (settings->normalHandles) {
        gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals_.size(),
                         normals_.data(), GL_DYNAMIC_DRAW);
        QColor qCol = settings->style.normCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("lineColor"), col);
        gl->glPointSize(settings->drawPointRadius);
        gl->glDrawElements(GL_POINTS, vboSize_, GL_UNSIGNED_INT, nullptr);
        if (settings->selectedNormal > -1) {
            gl->glPointSize(settings->selectedPointRadius);
            QColor secQCol = settings->style.selectedNormCol;
            QVector3D secCol(secQCol.redF(), secQCol.greenF(), secQCol.blueF());
            shader->setUniformValue(shader->uniformLocation("lineColor"), secCol);
            gl->glDrawArrays(GL_POINTS, settings->selectedNormal, 1);

        }
    }

    gl->glBindVertexArray(0);

    shader->release();
}
