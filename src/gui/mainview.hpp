#pragma once

#include <QOpenGLDebugLogger>
#include <QOpenGLWidget>

#include "gui/renderers/conicrenderer.hpp"
#include "gui/renderers/curvenetrenderer.hpp"
#include "gui/renderers/curverenderer.hpp"
#include "util/vector.hpp"
#include "core/conics/conic.hpp"

class SubdivisionCurve;

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
    Q_OBJECT

public:
    explicit MainView(Settings &settings, QWidget *parent = nullptr);

    ~MainView() override;

    void recalculateCurve();

    void recalculateNormals();

    void refineNormals();

    void refineSelectedNormal();

    void setControlCurve(Curve curve);

    void updateBuffers();

    void subdivideCurve(int numSteps);

    void paintGL() override;

    bool saveCurve(const char *fileName, const Curve &curve);
    bool saveCurveWithNormals(const char *fileName, const Curve &curve);

    Curve& getControlCurve() { return controlCurve_; }
    const Curve& getSubdivCurve() const { return subdivCurve_; }

protected:
    void initializeGL() override;

    Vector2DD toNormalizedScreenCoordinates(double x, double y);

    void mousePressEvent(QMouseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    Settings &settings_;
    Curve controlCurve_;
    Curve subdivCurve_;
    int lastSubdivLevel_ = 0;

    QOpenGLDebugLogger *debugLogger_ = nullptr;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;
    ConicRenderer conicR_;

    bool dragging_ = false;

    Vector2DD oldMouseCoords_;
    QMatrix4x4 toWorldCoordsMatrix_;

    bool attemptNormalHighlight(const Vector2DD &scenePos);

    bool attemptVertexHighlight(const Vector2DD &scenePos);

    void updateCursor(const Qt::KeyboardModifiers &flags);

    void resetViewMatrix();

    void translationUpdate(const Vector2DD &scenePos, const QPointF &mousePos);

    Conic getConicAtIndex(int idx) const;

    int selectedConicIdx_ = -1;

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);
};
