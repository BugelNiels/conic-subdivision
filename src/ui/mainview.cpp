#include "mainview.hpp"

#include <QOpenGLVersionFunctionsFactory>
#include "src/core/subdivisioncurve.hpp"
#include <QMouseEvent>
#include "core/settings.hpp"

MainView::MainView(Settings *settings, QWidget *parent) : QOpenGLWidget(parent), settings_(settings), cnr_(settings),
                                                          cr_(settings) {

    subCurve_ = std::make_shared<SubdivisionCurve>(SubdivisionCurve());
    QSurfaceFormat format;
    format.setSamples(4);    // Set the number of samples used for multisampling
    setFormat(format);
}


MainView::~MainView() {
    debugLogger_->stopLogging();

    delete debugLogger_;
}

void MainView::initializeGL() {
    initializeOpenGLFunctions();
    qDebug() << ":: OpenGL initialized";

    debugLogger_ = new QOpenGLDebugLogger();
    connect(debugLogger_, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(onMessageLogged(QOpenGLDebugMessage)),
            Qt::DirectConnection);

    if (debugLogger_->initialize()) {
        qDebug() << ":: Logging initialized";
        debugLogger_->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        debugLogger_->enableMessages();
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
    glEnable(GL_MULTISAMPLE);

    // grab the opengl context
    auto *functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>(this->context());
    // initialize renderers here with the current context
    cnr_.init(functions);
    cr_.init(functions);

    // update buffers
    updateBuffers();
}

void MainView::resizeGL(int width, int height) {
    QMatrix4x4 projectMatrix;
    float halfWidth = width / 2.0f / settings_->sizeCorrection;
    float halfHeight = height / 2.0f / settings_->sizeCorrection;
    projectMatrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0, 1);
    settings_->projectionMatrix = projectMatrix;
}

void MainView::subdivideCurve(int numSteps) {
    if (subCurve_ == nullptr) {
        return;
    }
    subCurve_->subdivide(numSteps);
    updateBuffers();
}

void MainView::recalculateCurve() {
    if (subCurve_ == nullptr) {
        return;
    }
    subCurve_->reSubdivide();
    updateBuffers();
}

/**
 * @brief MainView::updateBuffers Updates the buffers of the control net and
 * subdivision curve renderers.
 */
void MainView::updateBuffers() {
    if (subCurve_ == nullptr) {
        return;
    }
    cnr_.updateBuffers(*subCurve_);
    cr_.updateBuffers(*subCurve_);

    update();
}

/**
 * @brief MainView::paintGL Draw call.
 */
void MainView::paintGL() {
    QColor bCol = settings_->style.backgroundCol;
    glClearColor(bCol.redF(), bCol.greenF(), bCol.blueF(), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (subCurve_ != nullptr) {
        cr_.draw();
        cnr_.draw();
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
    if (subCurve_ == nullptr) {
        return;
    }
    QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
    switch (event->buttons()) {
        case Qt::LeftButton: {
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                // First attempt to select a normal. If unsuccessful, select a point
                if (!attemptNormalSelect(scenePos)) {
                    attemptVertexSelect(scenePos);
                }

            } else {
                // First attempt to select a vertex. If unsuccessful, select a normal
                if (!attemptVertexSelect(scenePos)) {
                    attemptNormalSelect(scenePos);
                }
            }
            update();
            break;
        }
        case Qt::RightButton: {
            // Add new control point
            int idx = subCurve_->addPoint(scenePos);
            settings_->selectedNormal = -1;
            settings_->selectedVertex = idx;
            updateBuffers();
            break;
        }
    }
}

bool MainView::attemptVertexSelect(const QVector2D &scenePos) {
    settings_->selectedNormal = -1;
    float maxDist = settings_->selectRadius;
    if (settings_->selectedVertex != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = settings_->deselectRadius;
    }
    // Select control point
    settings_->selectedVertex = subCurve_->findClosestVertex(scenePos, maxDist);
    if (settings_->selectedVertex > -1) {
        return true;
    }
    return false;
}

bool MainView::attemptNormalSelect(const QVector2D &scenePos) {
    settings_->selectedVertex = -1;
    float maxDist = settings_->selectRadius;
    if (settings_->selectedNormal != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = settings_->deselectRadius;
    }
    // Select control point
    settings_->selectedNormal = subCurve_->findClosestNormal(scenePos, maxDist);
    if (settings_->selectedNormal > -1) {
        return true;
    }
    return false;
}

/**
 * @brief MainView::mouseMoveEvent Handles the mouse moving. Used for changing
 * the position of the selected control point.
 * @param event Mouse event.
 */
void MainView::mouseMoveEvent(QMouseEvent *event) {
    if (settings_->selectedVertex > -1) {
        QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
        // Update position of the control point
        subCurve_->setVertexPosition(settings_->selectedVertex, scenePos);
        updateBuffers();
    }
    if (settings_->selectedNormal > -1) {
        QVector2D scenePos = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
        // Update position of the control point
        subCurve_->setNormalPosition(settings_->selectedNormal, scenePos);
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
    if (subCurve_ == nullptr) {
        return;
    }
    // Only works when the widget has focus!
    switch (event->key()) {
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_X:
            if (settings_->selectedVertex > -1) {
                // Remove selected control point
                subCurve_->removePoint(settings_->selectedVertex);
                settings_->selectedVertex = -1;
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
    subCurve_ = newSubCurve;
}

void MainView::flipCurveNorms() {
    if (subCurve_ == nullptr) {
        return;
    }
    subCurve_->flipNormals();
    recalculateCurve();
}

void MainView::recalculateNormals() {
    if (subCurve_ == nullptr) {
        return;
    }
    subCurve_->recalculateNormals();
    recalculateCurve();
}

const std::shared_ptr<SubdivisionCurve> &MainView::getSubCurve() const {
    return subCurve_;
}

void MainView::mouseDoubleClickEvent(QMouseEvent *event) {
    QWidget::mouseDoubleClickEvent(event);
    if(subCurve_ == nullptr || settings_->selectedNormal < 0) {
        return;
    }
    subCurve_->recalculateNormal(settings_->selectedNormal);
    recalculateCurve();
}
