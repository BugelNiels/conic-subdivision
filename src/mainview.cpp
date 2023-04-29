#include "mainview.h"

#include <QOpenGLVersionFunctionsFactory>

#include "math.h"

/**
 * @brief MainView::MainView Creates a new main view.
 * @param parent QT parent widget.
 */
MainView::MainView(QWidget *parent) : QOpenGLWidget(parent) {}

/**
 * @brief MainView::~MainView Deconstructs the main view.
 */
MainView::~MainView() {
  debugLogger->stopLogging();

  delete debugLogger;
}

/**
 * @brief MainView::initializeGL Initializes the opengl functions and settings,
 * initialises the renderers and sets up the debugger.
 */
void MainView::initializeGL() {
  initializeOpenGLFunctions();
  qDebug() << ":: OpenGL initialized";

  debugLogger = new QOpenGLDebugLogger();
  connect(debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
          SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

  if (debugLogger->initialize()) {
    qDebug() << ":: Logging initialized";
    debugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    debugLogger->enableMessages();
  }

  // If the application crashes here, try setting "MESA_GL_VERSION_OVERRIDE
  // = 4.1" and "MESA_GLSL_VERSION_OVERRIDE = 410" in Projects (left panel) ->
  // Build Environment

  QString glVersion;
  glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
  qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

  // Enable depth buffer
  glEnable(GL_DEPTH_TEST);
  // Default is GL_LESS
  glDepthFunc(GL_LEQUAL);

  // grab the opengl context
  QOpenGLFunctions_4_1_Core *functions =
      QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>(
          this->context());

  // initialize renderers here with the current context
  cnr.init(functions, &settings);
  cr.init(functions, &settings);

  subCurve.addSettings(&settings);
  // load preset curve net
  subCurve.presetNet(0);

  initialized = true;
  // update buffers
  updateBuffers();
}

void MainView::recalculateCurve() {
  if (!initialized) {
    return;
  }
  subCurve.reSubdivide();
  cnr.updateBuffers(subCurve, settings.closed);
  cr.updateBuffers(subCurve, settings.closed);
  update();
}

/**
 * @brief MainView::updateBuffers Updates the buffers of the control net and
 * subdivision curve renderers.
 */
void MainView::updateBuffers() {
  if (!initialized) {
    return;
  }
  cnr.updateBuffers(subCurve, settings.closed);
  cr.updateBuffers(subCurve, settings.closed);

  update();
}

/**
 * @brief MainView::paintGL Draw call.
 */
void MainView::paintGL() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (settings.showNet) {
    cnr.draw();
    cr.draw();
  }
}

/**
 * @brief MainView::toNormalizedScreenCoordinates Normalizes the mouse
 * coordinates to screen coordinates.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return A vector containing the normalized x and y screen coordinates.
 */
QVector2D MainView::toNormalizedScreenCoordinates(float x, float y) {
  float xRatio = x / float(width());
  float yRatio = y / float(height());

  // By default, the drawing canvas is the square [-1,1]^2:
  float xScene = (1 - xRatio) * -1 + xRatio * 1;
  // Note that the origin of the canvas is in the top left corner (not the lower
  // left).
  float yScene = yRatio * -1 + (1 - yRatio) * 1;

  return {xScene, yScene};
}

/**
 * @brief MainView::mousePressEvent Handles presses by the mouse. Left mouse
 * button moves a point. Right mouse button adds a new point at the click
 * location.
 * @param event Mouse event.
 */
void MainView::mousePressEvent(QMouseEvent *event) {
  // In order to allow keyPressEvents:
  setFocus();

  QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                     event->position().y());

  switch (event->buttons()) {
    case Qt::LeftButton: {
      if (hasMouseTracking()) {
        settings.selectedPoint = -1;
        setMouseTracking(false);
      }
      float maxDist = 0.6f;
      if (settings.selectedPoint != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = 0.1f;
      }
      // Select control point
      settings.selectedPoint = subCurve.findClosest(scenePos, maxDist);
      update();
      break;
    }
    case Qt::RightButton: {
      // Add new control point
      subCurve.addPoint(scenePos);
      settings.selectedPoint = -1;
      updateBuffers();
      break;
    }
  }
}

/**
 * @brief MainView::mouseMoveEvent Handles the mouse moving. Used for changing
 * the position of the selected control point.
 * @param event Mouse event.
 */
void MainView::mouseMoveEvent(QMouseEvent *event) {
  if (settings.selectedPoint > -1) {
    QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                       event->position().y());
    // Update position of the control point
    subCurve.setPointPosition(settings.selectedPoint, scenePos);
    updateBuffers();
  }
}

/**
 * @brief MainView::keyPressEvent Handles key press events. Used for keyboard
 * shortcuts. Note that, due to the way the UI works, the settings seen in the
 * UI might not be updated when a key is pressed.
 * @param event Key press event.
 */
void MainView::keyPressEvent(QKeyEvent *event) {
  // Only works when the widget has focus!
  switch (event->key()) {
    case 'G':
      if (settings.selectedPoint > -1) {
        // Grab selected control point
        setMouseTracking(true);
      }
      break;
    case 'X':
      if (settings.selectedPoint > -1) {
        // Remove selected control point
        subCurve.removePoint(settings.selectedPoint);
        settings.selectedPoint = -1;
        updateBuffers();
      }
      break;
  }
}

/**
 * @brief MainView::onMessageLogged Helper function for logging messages.
 * @param message The message to log.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage message) {
  bool log = false;

  // Format based on source
#define CASE(c) \
  case QOpenGLDebugMessage::c: :
  switch (message.source()) {
    case QOpenGLDebugMessage::APISource:
    case QOpenGLDebugMessage::WindowSystemSource:
    case QOpenGLDebugMessage::ThirdPartySource:
    case QOpenGLDebugMessage::ApplicationSource:
    case QOpenGLDebugMessage::OtherSource:
    case QOpenGLDebugMessage::InvalidSource:
    case QOpenGLDebugMessage::AnySource: {
      log = true;
      break;
    }
    case QOpenGLDebugMessage::ShaderCompilerSource:
      break;
  }
#undef CASE

  if (log) qDebug() << "  → Log:" << message;
}
