#include "curverenderer.hpp"

#include "conis/gui/util/colormap.hpp"

namespace conis::gui {

static const int COORDS_IDX = 0;
static const int NORM_IDX = 1;
static const int DOUBLE_IDX = 2;

CurveRenderer::CurveRenderer(const ViewSettings &settings) : Renderer(settings) {}

CurveRenderer::~CurveRenderer() {
    gl_->glDeleteVertexArrays(1, &vao_);

    gl_->glDeleteBuffers(numBuffers_, vbo_.data());
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
    vbo_.resize(numBuffers_);
    gl_->glGenBuffers(numBuffers_, vbo_.data());

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[COORDS_IDX]);
    gl_->glEnableVertexAttribArray(COORDS_IDX);
    gl_->glVertexAttribPointer(COORDS_IDX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[NORM_IDX]);
    gl_->glEnableVertexAttribArray(NORM_IDX);
    gl_->glVertexAttribPointer(NORM_IDX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

#ifdef SHADER_DOUBLE_PRECISION
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[DOUBLE_IDX]);
    gl_->glEnableVertexAttribArray(DOUBLE_IDX);
    gl_->glVertexAttribLPointer(DOUBLE_IDX, 2, GL_DOUBLE, 0, nullptr);
#endif

    gl_->glGenBuffers(1, &ibo_);
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    ColorMap colMap;
    auto colMapVals = colMap.getColorMap(settings_.style.colorMapName);

    texture_ = new QOpenGLTexture(QOpenGLTexture::Target1D);
    texture_->setFormat(QOpenGLTexture::RGB32F); // Adjust the format as needed
    texture_->setSize(colMapVals.size());
    texture_->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::Float32);
    texture_->setData(QOpenGLTexture::RGB, QOpenGLTexture::Float32, colMapVals.data());
    texture_->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_->bind();

    // unbind
    gl_->glBindVertexArray(0);
}

void CurveRenderer::updateBuffers(const conis::core::Curve &curve) {
    std::vector<QVector2D> coords = qVecToVec(curve.getCoords());
    std::vector<QVector2D> normals = qVecToVec(curve.getNormals());
    for (auto &norm: normals) {
        norm *= settings_.curvatureSign;
        norm.normalize();
    }
    int numVerts = coords.size();
    if (numVerts == 0) {
        vboSize_ = 0;
        return;
    }

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[COORDS_IDX]);
    gl_->glBufferData(GL_ARRAY_BUFFER,
                      sizeof(QVector2D) * numVerts,
                      coords.data(),
                      GL_DYNAMIC_DRAW);

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[NORM_IDX]);
    gl_->glBufferData(GL_ARRAY_BUFFER,
                      sizeof(QVector2D) * normals.size(),
                      normals.data(),
                      GL_DYNAMIC_DRAW);

#ifdef SHADER_DOUBLE_PRECISION
    const auto &coords_d = curve.getCoords();

    std::vector<double> doubleData;
    doubleData.reserve(coords_d.size() * 2);
    for (const auto &v: coords_d) {
        doubleData.push_back(double(v.x()));
        doubleData.push_back(double(v.y()));
    }

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_[DOUBLE_IDX]);
    gl_->glBufferData(GL_ARRAY_BUFFER,
                      sizeof(double) * doubleData.size(),
                      doubleData.data(),
                      GL_DYNAMIC_DRAW);
#endif

    std::vector<int> indices;
    indices.reserve(numVerts + 2);
    indices.emplace_back(curve.isClosed() ? numVerts - 1 : 0);
    for (int i = 0; i < numVerts; i++) {
        indices.emplace_back(i);
    }
    if (curve.isClosed()) {
        indices.emplace_back(0);
        indices.emplace_back(1);
    } else {
        indices.emplace_back(numVerts - 1);
    }
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                      sizeof(int) * indices.size(),
                      indices.data(),
                      GL_DYNAMIC_DRAW);

    vboSize_ = indices.size();
}

void CurveRenderer::draw() {
    if (vboSize_ == 0) {
        return;
    }
    auto shader = shaders_[ShaderType::POLYLINE];
    shader->bind();
    shader->setUniformValue("visualize_normals", settings_.visualizeNormals);
    shader->setUniformValue("visualize_curvature", settings_.visualizeCurvature);
    shader->setUniformValue("viewMatrix", settings_.viewMatrix);
    shader->setUniformValue("projectionMatrix", settings_.projectionMatrix);
    shader->setUniformValue("curvatureScale", settings_.curvatureScale);

    shader->setUniformValue("colorMap", 0);

    gl_->glLineWidth(settings_.curveLineWidth);
    QColor qCol = settings_.style.smoothCurveCol;
    QVector3D col(qCol.redF(), qCol.greenF(), qCol.blueF());
    shader->setUniformValue("lineColor", col);

    QColor normQCol = settings_.style.normCol;
    QVector3D normCol(normQCol.redF(), normQCol.greenF(), normQCol.blueF());
    shader->setUniformValue("normalColor", normCol);

    gl_->glBindVertexArray(vao_);

    gl_->glDrawElements(GL_LINE_STRIP_ADJACENCY, vboSize_, GL_UNSIGNED_INT, nullptr);

    shader->release();
}

} // namespace conis::gui