#include "renderer.hpp"

/**
 * @brief Renderer::Renderer Creates a new renderer.
 */
Renderer::Renderer(Settings *settings) : gl(nullptr), settings(settings) {}

/**
 * @brief Renderer::~Renderer Deconstructs the renderer by deleting all shaders.
 */
Renderer::~Renderer() { qDeleteAll(shaders); }

/**
 * @brief Renderer::init Initialises the renderer with an OpenGL context and
 * settings. Also initialises the shaders and buffers.
 * @param f OpenGL functions pointer.
 * @param s Settings.
 */
void Renderer::init(QOpenGLFunctions_4_1_Core *f) {
    gl = f;

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
QOpenGLShaderProgram *Renderer::constructPolyLineShader() const {
    QString pathVert = ":/shaders/polyline.vert";
    QString pathGeom = ":/shaders/polyline_normals.geom";
    QString pathFrag = ":/shaders/polyline.frag";

    // we use the qt wrapper functions for shader objects
    QOpenGLShaderProgram *shader = new QOpenGLShaderProgram();
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
QOpenGLShaderProgram *Renderer::constructDefaultShader(
        const QString &name) const {
    QString pathVert = ":/shaders/" + name + ".vert";
    QString pathFrag = ":/shaders/" + name + ".frag";

    // we use the qt wrapper functions for shader objects
    QOpenGLShaderProgram *shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, pathVert);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, pathFrag);
    shader->link();
    return shader;
}
