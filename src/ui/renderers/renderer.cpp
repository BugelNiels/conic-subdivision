#include "renderer.hpp"
#include "util/vector.hpp"

/**
 * @brief Renderer::Renderer Creates a new renderer.
 */
Renderer::Renderer(const Settings &settings) : gl_(nullptr), settings_(settings) {}

/**
 * @brief Renderer::~Renderer Deconstructs the renderer by deleting all shaders.
 */
Renderer::~Renderer() { qDeleteAll(shaders_); }

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
QOpenGLShaderProgram *Renderer::constructPolyLineShader() const {
    QString pathVert = ":/shaders/polyline.vert";
    QString pathGeom = ":/shaders/polyline_normals.geom";
    QString pathFrag = ":/shaders/polyline.frag";

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
QOpenGLShaderProgram *Renderer::constructDefaultShader(
        const QString &name) const {
    QString pathVert = ":/shaders/" + name + ".vert";
    QString pathFrag = ":/shaders/" + name + ".frag";

    // we use the qt wrapper functions for shader objects
    auto *shader = new QOpenGLShaderProgram();
    shader->addShaderFromSourceFile(QOpenGLShader::Vertex, pathVert);
    shader->addShaderFromSourceFile(QOpenGLShader::Fragment, pathFrag);
    shader->link();
    return shader;
}

QVector<QVector2D> Renderer::qVecToVec(const QVector<Vector2DD>& items) const {
    QVector<QVector2D> qItems;
    qItems.reserve(items.size());
    for(auto& item : items) {
        qItems.emplaceBack(QVector2D(float(item.x()), float(item.y())));
    }
    return qItems;
}