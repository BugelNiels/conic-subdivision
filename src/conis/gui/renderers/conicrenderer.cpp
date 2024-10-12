#include "conicrenderer.hpp"

#include "conis/gui/util/colormap.hpp"

namespace conis::gui {

ConicRenderer::ConicRenderer(const ViewSettings &settings) : Renderer(settings) {}

ConicRenderer::~ConicRenderer() {
    gl_->glDeleteVertexArrays(1, &vao_);
    gl_->glDeleteBuffers(1, &vbo_);
    gl_->glDeleteBuffers(1, &ibo_);
}

void ConicRenderer::initShaders() {
    shaders_.insert(ShaderType::CONIC, constructDefaultShader("conic"));
}

void ConicRenderer::initBuffers() {
    gl_->glGenVertexArrays(1, &vao_);
    gl_->glBindVertexArray(vao_);

    gl_->glGenBuffers(1, &vbo_);
    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl_->glEnableVertexAttribArray(0);
    gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    gl_->glGenBuffers(1, &ibo_);
    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);

    gl_->glBindVertexArray(0);

    QVector<QVector3D> quad = {{-1.0, -1.0, 0.0}, {1.0, -1.0, 0.0}, {-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}};

    gl_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    gl_->glBufferData(GL_ARRAY_BUFFER, sizeof(QVector3D) * quad.size(), quad.data(), GL_STATIC_DRAW);

    QVector<unsigned int> meshIndices = {0, 1, 2, 1, 3, 2};

    gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * meshIndices.size(), meshIndices.data(), GL_STATIC_DRAW);
    vboSize_ = meshIndices.size();

    // unbind
    gl_->glBindVertexArray(0);
}

void ConicRenderer::updateBuffers(const conis::core::Matrix3DD &q) {

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            conicCoefs_(i, j) = qreal(q(i, j));
        }
    }
}

void ConicRenderer::draw() {
    gl_->glBindVertexArray(vao_);
    auto shader = shaders_[ShaderType::CONIC];
    shader->bind();

    shader->setUniformValue("toWorldMatrix", (settings_.projectionMatrix * settings_.viewMatrix).inverted());
    shader->setUniformValue("conic", conicCoefs_);
    shader->setUniformValue("width", settings_.conicWidth);

    gl_->glDrawElements(GL_TRIANGLES, vboSize_, GL_UNSIGNED_INT, nullptr);

    shader->release();
    gl_->glBindVertexArray(0);
}

} // namespace conis::gui
