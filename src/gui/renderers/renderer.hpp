#pragma once

#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>

#include "core/vector.hpp"
#include "gui/viewsettings.hpp"
#include "shadertypes.hpp"

namespace conics::gui {

#define SHADER_DOUBLE_PRECISION

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

    static QOpenGLShaderProgram *constructPolyLineShader();

    static std::vector<QVector2D> qVecToVec(const std::vector<conics::core::Vector2DD> &items);

    template<class T>
    static std::vector<T> qVecToVec(const std::vector<conics::core::Vector2DD> &items) {
        std::vector<T> qItems;
        qItems.reserve(items.size() * 2);
        for (auto &item: items) {
            qItems.emplace_back(T(item.x()));
            qItems.emplace_back(T(item.y()));
        }
        return qItems;
    }
};

} // namespace conics::gui