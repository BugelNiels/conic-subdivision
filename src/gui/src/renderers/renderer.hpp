#pragma once

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>

#include "conis/core/vector.hpp"
#include "viewsettings.hpp"
#include "shadertypes.hpp"

namespace conis::gui {

/**
 * @brief The Renderer class represents a generic renderer class. The class is
 * abstract and should only contain functionality that is applicable
 * to every renderer.
 */
class Renderer {
public:
    explicit Renderer(const ViewSettings &settings);

    virtual ~Renderer();

    void init(QOpenGLFunctions_4_1_Core *f);

protected:
    const ViewSettings &settings_;

    QMap<ShaderType, QOpenGLShaderProgram *> shaders_;
    QOpenGLFunctions_4_1_Core *gl_ = nullptr;

    virtual void initShaders() = 0;

    virtual void initBuffers() = 0;

    static QOpenGLShaderProgram *constructDefaultShader(const QString &name);

    static QOpenGLShaderProgram *constructGeomShader(const QString &name);

    static std::vector<QVector2D> qVecToVec(const std::vector<core::Vector2DD> &items);

};

} // namespace conis::gui
