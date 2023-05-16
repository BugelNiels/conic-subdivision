#include "curverenderer.hpp"

#include "core/settings.hpp"


CurveRenderer::CurveRenderer(Settings *settings) : Renderer(settings) {

}

CurveRenderer::~CurveRenderer() {
    gl->glDeleteVertexArrays(1, &vao_);
    gl->glDeleteBuffers(1, &vbo_coords_);
    gl->glDeleteBuffers(1, &vbo_norms_);
    gl->glDeleteBuffers(1, &ibo_);
}

void CurveRenderer::initShaders() {
    shaders.insert(ShaderType::POLYLINE, constructPolyLineShader());
}

void CurveRenderer::initBuffers() {
    // Pure OpenGL functions used here

    // create vao
    gl->glGenVertexArrays(1, &vao_);
    // bind vao
    gl->glBindVertexArray(vao_);

    gl->glGenBuffers(1, &vbo_coords_);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl->glGenBuffers(1, &vbo_norms_);
    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms_);
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl->glGenBuffers(1, &ibo_);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    // unbind
    gl->glBindVertexArray(0);
}

void CurveRenderer::updateBuffers(SubdivisionCurve &sc) {
    QVector<QVector2D> coords;
    QVector<QVector2D> normals;
    if (sc.getSubdivLevel() == 0) {
        coords = sc.getNetCoords();
        normals = sc.getNetNormals();
    } else {
        coords = sc.getCurveCoords();
        normals = sc.getCurveNormals();
    }
    if (coords.size() == 0) {
        vboSize_ = 0;
        return;
    }

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords.size(),
                     coords.data(), GL_DYNAMIC_DRAW);

    gl->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms_);
    gl->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals.size(),
                     normals.data(), GL_DYNAMIC_DRAW);

    QVector<int> indices(coords.size() + 2);
    for (int i = 0; i < coords.size(); i++) {
        indices.append(i);
    }
    if (sc.isClosed()) {
        indices.prepend(coords.size() - 1);
        indices.append(0);
    } else {
        indices.prepend(0);
        indices.append(coords.size() - 1);
    }

    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                     indices.data(), GL_DYNAMIC_DRAW);

    vboSize_ = indices.size();
}

void CurveRenderer::draw() {
    // Always renders the control net using the flat shader.
    auto shader = shaders[ShaderType::POLYLINE];
    shader->bind();
    shader->setUniformValue(shader->uniformLocation("visualize_normals"),
                            settings->visualizeNormals);
    shader->setUniformValue(shader->uniformLocation("viewMatrix"),
                            settings->viewMatrix);

    gl->glLineWidth(settings->curveLineWidth);
    QColor qCol = settings->style.smoothCurveCol;
    QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
    shader->setUniformValue(shader->uniformLocation("lineColor"), col);

    QColor normQCol = settings->style.normCol;
    QVector3D normCol(normQCol.redF(), normQCol.greenF(), normQCol.blueF());
    shader->setUniformValue(shader->uniformLocation("normalColor"), normCol);

    shader->setUniformValue("projectionMatrix", settings->projectionMatrix);

    gl->glBindVertexArray(vao_);

    gl->glDrawElements(GL_LINE_STRIP_ADJACENCY, vboSize_, GL_UNSIGNED_INT,
                       nullptr);

    shader->release();
}
