#include "mainview.hpp"

#include <QOpenGLVersionFunctionsFactory>

#include "math.h"


MainView::MainView(Settings *settings, QWidget *parent) : QOpenGLWidget(parent), settings(settings) {

    subCurve = std::make_shared<SubdivisionCurve>(SubdivisionCurve());
    QSurfaceFormat format;
    format.setSamples(4);    // Set the number of samples used for multisampling
    setFormat(format);
}


MainView::~MainView() {
    debugLogger->stopLogging();

    delete debugLogger;
}

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
    auto *functions =
            QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>(
                    this->context());
    functions->glEnable(GL_MULTISAMPLE);

    functions->glLineWidth(2);

    // initialize renderers here with the current context
    cnr.init(functions, settings);
    cr.init(functions, settings);
    // update buffers
    updateBuffers();
}

void MainView::resizeGL(int width, int height) {
    QMatrix4x4 projectMatrix;
    float sizeCorrection = 500;
    float halfWidth = width / 2.0f / sizeCorrection;
    float halfHeight = height / 2.0f / sizeCorrection;
    projectMatrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0, 1);
    settings->projectionMatrix = projectMatrix;
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void MainView::subdivideCurve(int numSteps) {
    if (subCurve == nullptr) {
        return;
    }
    subCurve->subdivide(numSteps);
    cnr.updateBuffers(*subCurve, settings->closed);
    cr.updateBuffers(*subCurve, settings->closed);
    update();
}

void MainView::recalculateCurve() {
    if (subCurve == nullptr) {
        return;
    }
    // TODO: do this better
    subCurve->reSubdivide(settings->presetIdx < 3);
    cnr.updateBuffers(*subCurve, settings->closed);
    cr.updateBuffers(*subCurve, settings->closed);
    update();
}

/**
 * @brief MainView::updateBuffers Updates the buffers of the control net and
 * subdivision curve renderers.
 */
void MainView::updateBuffers() {
    if (subCurve == nullptr) {
        return;
    }
    cnr.updateBuffers(*subCurve, settings->closed);
    cr.updateBuffers(*subCurve, settings->closed);

    update();
}

/**
 * @brief MainView::paintGL Draw call.
 */
void MainView::paintGL() {
    QColor bCol = settings->style.backgroundCol;
    glClearColor(bCol.redF(), bCol.greenF(), bCol.blueF(), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (settings->showNet && subCurve != nullptr) {
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
    int side = qMin(width(), height());
    float xViewport = (width() - side) / 2.0f;
    float yViewport = (height() - side) / 2.0f;

    float xRatio = (x - xViewport) / side;
    float yRatio = (y - yViewport) / side;
    float xScene = (1 - xRatio) * -1 + xRatio * 1;
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
    if (subCurve == nullptr) {
        return;
    }

    QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                       event->position().y());

    switch (event->buttons()) {
        case Qt::LeftButton: {
            if (hasMouseTracking()) {
                settings->selectedVertex = -1;
                setMouseTracking(false);
            }
            float maxDist = 0.6f;
            if (settings->selectedVertex != -1) {
                // Smaller click radius for easy de-selection of points
                maxDist = 0.1f;
            }
            // Select control point
            settings->selectedVertex = subCurve->findClosest(scenePos, maxDist);
            update();
            break;
        }
        case Qt::RightButton: {
            // Add new control point
            subCurve->addPoint(scenePos);
            settings->selectedVertex = -1;
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
    if (settings->selectedVertex > -1) {
        QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                           event->position().y());
        // Update position of the control point
        subCurve->setPointPosition(settings->selectedVertex, scenePos);
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
    if (subCurve == nullptr) {
        return;
    }
    // Only works when the widget has focus!
    switch (event->key()) {
        case 'G':
            if (settings->selectedVertex > -1) {
                // Grab selected control point
                setMouseTracking(true);
            }
            break;
        case 'X':
            if (settings->selectedVertex > -1) {
                // Remove selected control point
                subCurve->removePoint(settings->selectedVertex);
                settings->selectedVertex = -1;
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
    if (message.severity() == QOpenGLDebugMessage::LowSeverity || message.type() == QOpenGLDebugMessage::OtherType) {
        return;
    }

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

void MainView::setSubCurve(std::shared_ptr<SubdivisionCurve> newSubCurve) {
    subCurve = newSubCurve;
    subCurve->addSettings(settings);
}
