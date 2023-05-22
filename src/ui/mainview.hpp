#pragma once

#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>

#include "ui/renderers/curvenetrenderer.hpp"
#include "ui/renderers/curverenderer.hpp"

class SubdivisionCurve;

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
Q_OBJECT

public:
    explicit MainView(Settings &settings, QWidget *parent = nullptr);

    ~MainView() override;

    void recalculateCurve();

    void recalculateNormals();

    void setSubCurve(std::shared_ptr<SubdivisionCurve> subCurve);

    void updateBuffers();

    void subdivideCurve(int numSteps);

    void paintGL() override;

    const std::shared_ptr<SubdivisionCurve> &getSubCurve() const;

protected:
    void initializeGL() override;

    QVector2D toNormalizedScreenCoordinates(float x, float y);

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


    QOpenGLDebugLogger *debugLogger_ = nullptr;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;

    std::shared_ptr<SubdivisionCurve> subCurve_;

    bool dragging_ = false;

    QVector2D oldMouseCoords_;
    QMatrix4x4 toWorldCoordsMatrix_;

    bool attemptNormalSelect(const QVector2D &scenePos);

    bool attemptVertexSelect(const QVector2D &scenePos);

    void updateCursor(const Qt::KeyboardModifiers &flags);

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);


    void resetViewMatrix();

    void translationUpdate(const QVector2D &scenePos, const QPointF &mousePos);
};

