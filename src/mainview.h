#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector2D>

#include "renderers/curvenetrenderer.h"
#include "renderers/curverenderer.h"
#include "subdivisioncurve.h"

/**
 * @brief The MainView class represents the main view of the UI. It handles and
 * orchestrates the different renderers.
 */
class MainView : public QOpenGLWidget, protected QOpenGLFunctions_4_1_Core {
  Q_OBJECT

 public:
  MainView(QWidget *parent = nullptr);
  ~MainView() override;

  void recalculateCurve();

 protected:
  void initializeGL() override;
  void paintGL() override;

  void updateBuffers();

  QVector2D toNormalizedScreenCoordinates(float x, float y);

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

  void resizeGL(int width, int height) override;

 private:
  bool initialized = false;
  QOpenGLDebugLogger *debugLogger;

  // add some renderers here
  CurveNetRenderer cnr;
  CurveRenderer cr;

  // we make mainwindow a friend so it can access the subcurve and settings
  friend class MainWindow;
  Settings settings;
  SubdivisionCurve subCurve;
 private slots:
  void onMessageLogged(QOpenGLDebugMessage message);
};

#endif  // MAINVIEW_H
