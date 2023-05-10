#pragma once

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>

#include "shadertypes.hpp"

class Settings;

/**
 * @brief The Renderer class represents a generic renderer class. The class is
 * abstract and should only contain functionality that is applicable
 * to every renderer.
 */
class Renderer {
public:
    Renderer();

    Renderer(QOpenGLFunctions_4_1_Core *functions, Settings *settings);

    virtual ~Renderer();

    void init(QOpenGLFunctions_4_1_Core *f, Settings *s);

protected:
    virtual void initShaders() = 0;

    virtual void initBuffers() = 0;

    QOpenGLShaderProgram *constructDefaultShader(const QString &name) const;

    QOpenGLShaderProgram *constructPolyLineShader() const;

    QMap<ShaderType, QOpenGLShaderProgram *> shaders;
    QOpenGLFunctions_4_1_Core *gl;
    Settings *settings;
};
