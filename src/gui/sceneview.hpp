#pragma once

#include <QOpenGLDebugLogger>
#include <QOpenGLWidget>

#include "core/conics/conic.hpp"
#include "core/scene.hpp"
#include "core/scenelistener.hpp"
#include "core/vector.hpp"
#include "gui/renderers/conicrenderer.hpp"
#include "gui/renderers/curvenetrenderer.hpp"
#include "gui/renderers/curverenderer.hpp"
#include "gui/viewsettings.hpp"

namespace conics::gui {

class SceneView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core, public conics::core::SceneListener {
    Q_OBJECT

public:
    SceneView(ViewSettings &settings, conics::core::Scene &scene, QWidget *parent = nullptr);

    ~SceneView() override;

    void updateBuffers();

    void paintGL() override;
    void viewToFit();

    [[nodiscard]] inline const conics::core::Scene &getScene() const { return scene_; }
    [[nodiscard]] inline conics::core::Scene &getScene() { return scene_; }

    void sceneUpdated();

protected:
    void initializeGL() override;

    conics::core::Vector2DD toNormalizedScreenCoordinates(double x, double y);

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    conics::core::Scene &scene_;
    ViewSettings &settings_;

    QOpenGLDebugLogger *debugLogger_ = nullptr;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;
    ConicRenderer conicR_;

    bool dragging_ = false;
    int selectedConicIdx_ = -1;

    conics::core::Vector2DD oldMouseCoords_;
    QMatrix4x4 toWorldCoordsMatrix_;

    bool attemptNormalHighlight(const conics::core::Vector2DD &scenePos);

    bool attemptVertexHighlight(const conics::core::Vector2DD &scenePos);

    void updateCursor(const Qt::KeyboardModifiers &flags);

    void resetViewMatrix();

    void translationUpdate(const conics::core::Vector2DD &scenePos, const QPointF &mousePos);

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);
};

} // namespace conics::gui
