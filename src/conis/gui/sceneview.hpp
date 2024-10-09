#pragma once

#include <QOpenGLDebugLogger>
#include <QOpenGLWidget>

#include "conis/core/conics/conic.hpp"
#include "conis/core/coniscurve.hpp"
#include "conis/core/listener.hpp"
#include "conis/core/vector.hpp"
#include "conis/gui/renderers/conicrenderer.hpp"
#include "conis/gui/renderers/curvenetrenderer.hpp"
#include "conis/gui/renderers/curverenderer.hpp"
#include "conis/gui/viewsettings.hpp"

namespace conis::gui {

class SceneView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core, public conis::core::Listener {
    Q_OBJECT

public:
    SceneView(ViewSettings &settings, conis::core::ConisCurve &conisCurve, QWidget *parent = nullptr);

    ~SceneView() override;

    void updateBuffers();

    void paintGL() override;
    void viewToFit();

    [[nodiscard]] inline const conis::core::ConisCurve &getConisCurve() const { return conisCurve_; }
    [[nodiscard]] inline conis::core::ConisCurve &getConisCurve() { return conisCurve_; }

    void onListenerUpdated();

protected:
    void initializeGL() override;

    conis::core::Vector2DD toNormalizedScreenCoordinates(double x, double y);

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    conis::core::ConisCurve &conisCurve_;
    ViewSettings &settings_;

    QOpenGLDebugLogger *debugLogger_ = nullptr;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;
    ConicRenderer conicR_;

    bool dragging_ = false;
    int selectedConicIdx_ = -1;

    conis::core::Vector2DD oldMouseCoords_;
    QMatrix4x4 toWorldCoordsMatrix_;

    bool attemptNormalHighlight(const conis::core::Vector2DD &scenePos);

    bool attemptVertexHighlight(const conis::core::Vector2DD &scenePos);

    void updateCursor(const Qt::KeyboardModifiers &flags);

    void resetViewMatrix();

    void translationUpdate(const conis::core::Vector2DD &scenePos, const QPointF &mousePos);

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);
};

} // namespace conis::gui
