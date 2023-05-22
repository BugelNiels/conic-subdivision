#include "curverenderer.hpp"

#include "core/settings.hpp"
#include "util/colormap.hpp"


CurveRenderer::CurveRenderer(const Settings &settings) : Renderer(settings) {

}

CurveRenderer::~CurveRenderer() {
    gl_->glDeleteVertexArrays(1, &vao_);
    gl_->glDeleteBuffers(1, &vbo_coords_);
    gl_->glDeleteBuffers(1, &vbo_norms_);
    gl_->glDeleteBuffers(1, &vbo_stab_);
    gl_->glDeleteBuffers(1, &ibo_);
    delete texture_;
}

void CurveRenderer::initShaders() {
    shaders_.insert(ShaderType::POLYLINE, constructPolyLineShader());
}

void CurveRenderer::initBuffers() {
    // Pure OpenGL functions used here

    // create vao
    gl_->glGenVertexArrays(1, &vao_);
    // bind vao
    gl_->glBindVertexArray(vao_);

    gl_->glGenBuffers(1, &vbo_coords_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl_->glEnableVertexAttribArray(0);
    gl_->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glGenBuffers(1, &vbo_norms_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms_);
    gl_->glEnableVertexAttribArray(1);
    gl_->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glGenBuffers(1, &vbo_stab_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_stab_);
    gl_->glEnableVertexAttribArray(2);
    gl_->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glGenBuffers(1, &ibo_);
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);


    ColorMap colMap;
    auto colMapVals = colMap.getColorMap(settings_.style.colorMapName);

    texture_ = new QOpenGLTexture(QOpenGLTexture::Target1D);
    texture_->setFormat(QOpenGLTexture::RGB32F);  // Adjust the format as needed
    texture_->setSize(colMapVals.size());
    texture_->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float32);
    texture_->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, colMapVals.data());
    texture_->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_->bind();

    // unbind
    gl_->glBindVertexArray(0);
}

void CurveRenderer::updateBuffers(SubdivisionCurve &sc) {
    QVector<QVector2D> coords;
    QVector<QVector2D> normals;
    QVector<float> stability;
    if (sc.getSubdivLevel() == 0) {
        coords = sc.getNetCoords();
        normals = sc.getNetNormals();
        stability.resize(coords.size());
        stability.fill(0);
    } else {
        coords = sc.getCurveCoords();
        normals = sc.getCurveNormals();
        stability = sc.getStabilityVals();
    }
    if (coords.size() == 0) {
        vboSize_ = 0;
        return;
    }

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_coords_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * coords.size(),
                      coords.data(), GL_DYNAMIC_DRAW);

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_norms_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector2D) * normals.size(),
                      normals.data(), GL_DYNAMIC_DRAW);

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_stab_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * stability.size(),
                      stability.data(), GL_DYNAMIC_DRAW);

    QVector<int> indices(coords.size() + 2);
    for (int i = 0; i < coords.size(); i++) {
        indices.append(i);
    }
    if (sc.isClosed()) {
        indices.prepend(coords.size() - 1);
        indices.append(0);
        indices.append(1);
        if (coords.size() > 2) {
            indices.append(2);
        }
    } else {
        indices.prepend(0);
        indices.append(coords.size() - 1);
    }

    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(),
                      indices.data(), GL_DYNAMIC_DRAW);

    vboSize_ = indices.size();
}

void CurveRenderer::draw() {
    // Always renders the control net using the flat shader.
    auto shader = shaders_[ShaderType::POLYLINE];
    shader->bind();
    shader->setUniformValue(shader->uniformLocation("visualize_normals"),
                            settings_.visualizeNormals);
    shader->setUniformValue(shader->uniformLocation("visualize_curvature"),
                            settings_.visualizeCurvature);
    shader->setUniformValue(shader->uniformLocation("stability_colors"),
                            settings_.visualizeStability);
    shader->setUniformValue(shader->uniformLocation("viewMatrix"),
                            settings_.viewMatrix);

    shader->setUniformValue(shader->uniformLocation("colorMap"), 0);

    gl_->glLineWidth(settings_.curveLineWidth);
    QColor qCol = settings_.style.smoothCurveCol;
    QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
    shader->setUniformValue(shader->uniformLocation("lineColor"), col);

    QColor normQCol = settings_.style.normCol;
    QVector3D normCol(normQCol.redF(), normQCol.greenF(), normQCol.blueF());
    shader->setUniformValue(shader->uniformLocation("normalColor"), normCol);

    shader->setUniformValue("projectionMatrix", settings_.projectionMatrix);

    gl_->glBindVertexArray(vao_);

    gl_->glDrawElements(GL_LINE_STRIP_ADJACENCY, vboSize_, GL_UNSIGNED_INT,
                        nullptr);

    shader->release();
}
