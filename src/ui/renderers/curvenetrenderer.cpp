#include "curvenetrenderer.hpp"

/**
 * @brief CurveNetRenderer::~CurveNetRenderer Destructor of the control curve
 * net renderer.
 */
CurveNetRenderer::~CurveNetRenderer() {
    gl->glDeleteVertexArrays(1, &vao);
    gl->glDeleteBuffers(1, &vbo_coords);
    gl->glDeleteBuffers(1, &vbo_norms);
    gl->glDeleteBuffers(1, &ibo);
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
    gl->glGenVertexArrays(1, &vao);
    // bind vao
    gl->glBindVertexArray(vao);

    gl->glGenBuffers(1, &vbo_coords);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl->glGenBuffers(1, &vbo_norms);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms);
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl->glGenBuffers(1, &ibo);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // unbind
    gl->glBindVertexArray(0);
}

/**
 * @brief CurveNetRenderer::updateBuffers Updates the contents of the buffers.
 * @param sc The subdivision curve containing the information to update the
 * buffer(s) with.
 */
void CurveNetRenderer::updateBuffers(SubdivisionCurve &sc, bool closed) {
    QVector<QVector2D> coords = sc.getNetCoords();
    QVector<QVector2D> normals = sc.getNetNormals();
    if (coords.size() == 0) {
        return;
    }

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords.size(),
                     coords.data(), GL_DYNAMIC_DRAW);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals.size(),
                     normals.data(), GL_DYNAMIC_DRAW);


    QVector<int> indices(coords.size() + 2);
    for (int i = 0; i < coords.size(); i++) {
        indices.append(i);
    }
    if (closed) {
        indices.prepend(coords.size() - 1);
        indices.append(0);
    } else {
        indices.prepend(0);
        indices.append(coords.size() - 1);
    }

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                     indices.data(), GL_DYNAMIC_DRAW);

    vboSize = indices.size();
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

    gl->glLineWidth(1);
    gl->glBindVertexArray(vao);

    shader->setUniformValue("projectionMatrix", settings->projectionMatrix);

    if (settings->showControlCurve) {
        QColor qCol = settings->style.controlCurveCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("line_color"), col);
        gl->glDrawElements(GL_LINE_STRIP, vboSize, GL_UNSIGNED_INT, nullptr);
    }
    if (settings->showControlPoints) {
        QColor qCol = settings->style.controlPointCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());

        shader->setUniformValue(shader->uniformLocation("line_color"), col);
        gl->glPointSize(8.0);
        gl->glDrawElements(GL_POINTS, vboSize, GL_UNSIGNED_INT, nullptr);
    }

    // Highlight selected control point
    if (settings->selectedVertex > -1) {
        QColor qCol = settings->style.selectedCol;
        QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
        shader->setUniformValue(shader->uniformLocation("line_color"), col);
        gl->glPointSize(12.0);
        gl->glDrawArrays(GL_POINTS, settings->selectedVertex, 1);
    }

    gl->glBindVertexArray(0);

    shader->release();
}
