#pragma once

#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector2D>

#include "ui/renderers/curvenetrenderer.hpp"
#include "ui/renderers/curverenderer.hpp"
#include "src/core/subdivisioncurve.hpp"

class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
Q_OBJECT

public:
    MainView(Settings *settings, QWidget *parent = nullptr);

    ~MainView() override;

    void recalculateCurve();

    void setSubCurve(std::shared_ptr<SubdivisionCurve> subCurve);

    void updateBuffers();

    void subdivideCurve(int numSteps);
    void paintGL() override;

protected:
    void initializeGL() override;



    QVector2D toNormalizedScreenCoordinates(float x, float y);

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void resizeGL(int width, int height) override;

private:
    Settings *settings;
    QOpenGLDebugLogger *debugLogger;

    CurveNetRenderer cnr;
    CurveRenderer cr;

    std::shared_ptr<SubdivisionCurve> subCurve;

private slots:

    void onMessageLogged(QOpenGLDebugMessage message);

};

