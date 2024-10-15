#include "renderer.hpp"

namespace conis::gui {

/**
 * @brief Renderer::Renderer Creates a new renderer.
 */
Renderer::Renderer(const ViewSettings &settings) : settings_(settings) {}

/**
 * @brief Renderer::~Renderer Deconstructs the renderer by deleting all shaders.
 */
Renderer::~Renderer() {
    qDeleteAll(shaders_);
}

/**
 * @brief Renderer::init Initialises the renderer with an OpenGL context and
 * settings. Also initialises the shaders and buffers.
 * @param f OpenGL functions pointer.
 * @param s Settings.
 */
void Renderer::init(QOpenGLFunctions_4_1_Core *f) {
    gl_ = f;

    initShaders();
    initBuffers();
}

/**
 * @brief Renderer::constructDefaultShader Constructs a shader consisting of a
 * vertex shader and a fragment shader. The shaders are assumed to follow the
 * naming convention: <name>.vert and <name>.frag. Both of these files have to
 * exist for this function to work successfully.
 * @param name Name of the shader.
 * @return The constructed shader.
 */
QOpenGLShaderProgram *Renderer::constructGeomShader(const QString &name) {
#ifdef SHADER_DOUBLE_PRECISION
    QString pathVert = ":/shaders/double/" + name + ".vert";
    QString pathGeom = ":/shaders/double/" + name + ".geom";
    QString pathFrag = ":/shaders/double/" + name + ".frag";

#else
    QString pathVert = ":/shaders/" + name + ".vert";
    QString pathGeom = ":/shaders/" + name + ".geom";
    QString pathFrag = ":/shaders/" + name + ".frag";
#endif

    // we use the qt wrapper functions for shader objects
    auto *shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, pathVert);
    shader->addShaderFromSourceFile(QOpenGLShader::Geometry, pathGeom);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, pathFrag);
    shader->link();
    return shader;
}

/**
 * @brief Renderer::constructDefaultShader Constructs a shader consisting of a
 * vertex shader and a fragment shader. The shaders are assumed to follow the
 * naming convention: <name>.vert and <name>.frag. Both of these files have to
 * exist for this function to work successfully.
 * @param name Name of the shader.
 * @return The constructed shader.
 */
QOpenGLShaderProgram *Renderer::constructDefaultShader(const QString &name) {
    QString pathVert = ":/shaders/" + name + ".vert";
    QString pathFrag = ":/shaders/" + name + ".frag";

    // we use the qt wrapper functions for shader objects
    auto *shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, pathVert);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, pathFrag);
    shader->link();
    return shader;
}

std::vector<QVector2D> Renderer::qVecToVec(const std::vector<core::Vector2DD> &items) {
    std::vector<QVector2D> qItems;
    qItems.reserve(items.size());
    for (auto &item: items) {
        qItems.emplace_back(static_cast<float>(item.x()), static_cast<float>(item.y()));
    }
    return qItems;
}

}