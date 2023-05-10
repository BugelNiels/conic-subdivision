#pragma once

#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>

#include "ui/renderers/curvenetrenderer.hpp"
#include "ui/renderers/curverenderer.hpp"

class SubdivisionCurve;

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
Q_OBJECT

public:
    MainView(Settings *settings, QWidget *parent = nullptr);

    ~MainView() override;

    void recalculateCurve();

    void recalculateNormals();

    void setSubCurve(std::shared_ptr<SubdivisionCurve> subCurve);

    void updateBuffers();

    void subdivideCurve(int numSteps);

    void flipCurveNorms();

    void paintGL() override;

    const std::shared_ptr<SubdivisionCurve> &getSubCurve() const;

protected:
    void initializeGL() override;

    QVector2D toNormalizedScreenCoordinates(float x, float y);

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    bool attemptNormalSelect(const QVector2D &scenePos);

    bool attemptVertexSelect(const QVector2D &scenePos);

    Settings *settings_;
    QOpenGLDebugLogger *debugLogger_;

    CurveNetRenderer cnr_;
    CurveRenderer cr_;

    std::shared_ptr<SubdivisionCurve> subCurve_;

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);

};

