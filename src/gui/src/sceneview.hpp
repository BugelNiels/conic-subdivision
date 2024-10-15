#pragma once

#include <QOpenGLDebugLogger>
#include <QOpenGLWidget>

#include "conis/core/coniscurve.hpp"
#include "conis/core/listener.hpp"
#include "conis/core/vector.hpp"
#include "renderers/conicrenderer.hpp"
#include "renderers/curvenetrenderer.hpp"
#include "renderers/curverenderer.hpp"
#include "viewsettings.hpp"

namespace conis::gui {

class SceneView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core, public core::Listener {
    Q_OBJECT

public:
    SceneView(ViewSettings &settings, core::ConisCurve &conisCurve, QWidget *parent = nullptr);

    ~SceneView() override;

    void updateBuffers();

    void paintGL() override;
    void viewToFit();

    [[nodiscard]] const core::ConisCurve &getConisCurve() const { return conisCurve_; }
    [[nodiscard]] core::ConisCurve &getConisCurve() { return conisCurve_; }

    void onListenerUpdated();

protected:
    void initializeGL() override;

    conis::core::Vector2DD toNormalizedScreenCoordinates(double x, double y) const;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    core::ConisCurve &conisCurve_;
    ViewSettings &settings_;

    std::unique_ptr<QOpenGLDebugLogger> debugLogger_ = nullptr;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;
    ConicRenderer conicR_;

    bool dragging_ = false;

    core::Vector2DD oldMouseCoords_;
    QMatrix4x4 toWorldCoordsMatrix_;

    bool attemptNormalHighlight(const core::Vector2DD &scenePos);
    bool attemptVertexHighlight(const core::Vector2DD &scenePos);
    bool attemptEdgeHighlight(const core::Vector2DD &scenePos);

    void unHighlightAll();
    void highlightNormal(int idx);
    void highlightVertex(int idx);
    void highlightEdge(int idx);

    void selectVertex(int idx);
    void selectEdge(int idx);

    void updateSelectedConic();

    void updateCursor(const Qt::KeyboardModifiers &flags);

    void resetViewMatrix();

    void translationUpdate(const conis::core::Vector2DD &scenePos, const QPointF &mousePos);

private slots:

    static void onMessageLogged(const QOpenGLDebugMessage &message);
};

} // namespace conis::gui
