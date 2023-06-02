#pragma once

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>

#include "shadertypes.hpp"
#include "util/vector.hpp"

//#define SHADER_DOUBLE_PRECISION

class Settings;

/**
 * @brief The Renderer class represents a generic renderer class. The class is
 * abstract and should only contain functionality that is applicable
 * to every renderer.
 */
class Renderer {
public:
    explicit Renderer(const Settings &settings);

    virtual ~Renderer();

    void init(QOpenGLFunctions_4_1_Core *f);

protected:
    const Settings &settings_;

    QMap<ShaderType, QOpenGLShaderProgram *> shaders_;
    QOpenGLFunctions_4_1_Core *gl_ = nullptr;

    virtual void initShaders() = 0;

    virtual void initBuffers() = 0;

    static QOpenGLShaderProgram *constructDefaultShader(const QString &name) ;

    static QOpenGLShaderProgram *constructPolyLineShader() ;

    static QVector<QVector2D> qVecToVec(const QVector<Vector2DD> &items) ;
};
