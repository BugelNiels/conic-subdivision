#include "mainview.hpp"

#include <Eigen/Core>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLVersionFunctionsFactory>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

#include "core/settings/settings.hpp"
#include "core/subdivision/conicsubdivider.hpp"
#include "util/vector.hpp"

MainView::MainView(Settings &settings, QWidget *parent)
    : QOpenGLWidget(parent),
      settings_(settings),
      cnr_(settings),
      cr_(settings),
      conicR_(settings) {
    setMinimumWidth(200);
    QSurfaceFormat format;
    format.setSamples(4); // Set the number of samples used for multisampling
    setFormat(format);
    resetViewMatrix();
    setMouseTracking(true);
}

MainView::~MainView() {
    debugLogger_->stopLogging();
    delete debugLogger_;
}

void MainView::initializeGL() {
    initializeOpenGLFunctions();
    debugLogger_ = new QOpenGLDebugLogger();
    connect(debugLogger_,
            SIGNAL(messageLogged(QOpenGLDebugMessage)),
            this,
            SLOT(onMessageLogged(QOpenGLDebugMessage)),
            Qt::DirectConnection);

    if (debugLogger_->initialize()) {
        debugLogger_->startLogging(QOpenGLDebugLogger::SynchronousLogging);
        debugLogger_->enableMessages();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);

    // grab the opengl context
    auto *functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>(
            this->context());
    cnr_.init(functions);
    cr_.init(functions);
    conicR_.init(functions);
    updateBuffers();
}

// TODO: handle all the controller stuff elsewhere
void MainView::resetViewMatrix() {
    settings_.viewMatrix.setToIdentity();
    settings_.viewMatrix.scale(settings_.initialScale);
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    update();
}

void MainView::resizeGL(int width, int height) {
    QMatrix4x4 projectMatrix;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    projectMatrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0, 1);
    settings_.projectionMatrix = projectMatrix;
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
}

void MainView::subdivideCurve(int numSteps) {
    // TODO: make more efficient
    lastSubdivLevel_ = numSteps;
    ConicSubdivider subdivider(settings_);
    subdivCurve_ = controlCurve_;
    subdivider.subdivide(subdivCurve_, numSteps);
    updateBuffers();
}

Conic MainView::getConicAtIndex(int idx) const {
    ConicSubdivider subdivider(settings_);
    std::vector<PatchPoint> patch = subdivider.extractPatch(controlCurve_,
                                                            idx,
                                                            settings_.patchSize);
    return Conic(patch, settings_);
}

void MainView::recalculateCurve() {
    if (selectedConicIdx_ >= 0 && selectedConicIdx_ < controlCurve_.numPoints()) {
        Matrix3DD selectedConic = getConicAtIndex(selectedConicIdx_).getMatrix();
        conicR_.updateBuffers(selectedConic);
    }
    subdivideCurve(lastSubdivLevel_);
}

/**
 * @brief MainView::updateBuffers Updates the buffers of the control net and
 * subdivision curve renderers.
 */
void MainView::updateBuffers() {
    cr_.updateBuffers(subdivCurve_);
    cnr_.updateBuffers(controlCurve_);

    update();
}

/**
 * @brief MainView::paintGL Draw call.
 */
void MainView::paintGL() {
    QColor bCol = settings_.style.backgroundCol;
    glClearColor(bCol.redF(), bCol.greenF(), bCol.blueF(), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cr_.draw();
    cnr_.draw();
    if (settings_.testToggle) {
        conicR_.draw();
    }
}

/**
 * @brief MainView::toNormalizedScreenCoordinates Normalizes the mouse
 * coordinates to screen coordinates.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return A vector containing the normalized x and y screen coordinates.
 */
Vector2DD MainView::toNormalizedScreenCoordinates(double x, double y) {
    double xRatio = (x - 0) / width();
    double yRatio = (y - 0) / height();

    double xScene = xRatio * 2.0f - 1.0f;
    double yScene = 1.0f - yRatio * 2.0f;
    QVector4D from = toWorldCoordsMatrix_ * QVector4D(xScene, yScene, 0, 1.0f);
    return Vector2DD(from.x(), from.y());
}

bool MainView::attemptVertexHighlight(const Vector2DD &scenePos) {
    settings_.highlightedNormal = -1;
    float maxDist = settings_.selectRadius;
    if (settings_.highlightedVertex != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = settings_.deselectRadius;
    }
    // Select control point
    settings_.highlightedVertex = controlCurve_.findClosestVertex(scenePos, maxDist);
    if (settings_.highlightedVertex > -1) {
        return true;
    }
    return false;
}

bool MainView::attemptNormalHighlight(const Vector2DD &scenePos) {
    float maxDist = settings_.selectRadius;
    if (settings_.highlightedNormal != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = settings_.deselectRadius;
    }
    // Select control point
    settings_.highlightedNormal = controlCurve_.findClosestNormal(scenePos,
                                                                  maxDist,
                                                                  settings_.normalLength);
    if (settings_.highlightedNormal > -1) {
        settings_.highlightedVertex = -1;
        return true;
    }
    return false;
}

void MainView::translationUpdate(const Vector2DD &scenePos, const QPointF &mousePos) {
    if (!dragging_) {
        dragging_ = true;
        oldMouseCoords_ = scenePos;
        return;
    }
    Vector2DD translationUpdate = (scenePos - oldMouseCoords_) * settings_.dragSensitivity;
    settings_.viewMatrix.translate(translationUpdate.x(), translationUpdate.y());
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    oldMouseCoords_ = toNormalizedScreenCoordinates(mousePos.x(), mousePos.y());
    update();
}

void MainView::mousePressEvent(QMouseEvent *event) {
    // In order to allow keyPressEvents:
    setFocus();
    Vector2DD scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                       event->position().y());
    switch (event->buttons()) {
        case Qt::LeftButton: {
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                int idx = controlCurve_.addPoint(scenePos);
                settings_.highlightedNormal = -1;
                settings_.highlightedVertex = idx;
                recalculateCurve();
            } else {
                // First attempt to select a vertex. If unsuccessful, select a normal
                if (!attemptVertexHighlight(scenePos)) {
                    attemptNormalHighlight(scenePos);
                } else {
                    settings_.selectedVertex = settings_.highlightedVertex;
                    Matrix3DD selectedConic = getConicAtIndex(settings_.highlightedVertex)
                                                      .getMatrix();
                    conicR_.updateBuffers(selectedConic);
                    selectedConicIdx_ = settings_.highlightedVertex;
                }
            }
            update();
            break;
        }
        case Qt::RightButton: {
            // Add new control point
            int idx = controlCurve_.addPoint(scenePos);
            settings_.highlightedNormal = -1;
            settings_.highlightedVertex = idx;
            recalculateCurve();
            break;
        }
        case Qt::MiddleButton: {
            setCursor(Qt::ClosedHandCursor);
            break;
        }
    }
}

void MainView::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    Vector2DD scenePos = toNormalizedScreenCoordinates(event->position().x(),
                                                       event->position().y());
    if (event->buttons() == Qt::LeftButton && event->modifiers().testFlag(Qt::ShiftModifier)) {
        setCursor(Qt::ClosedHandCursor);
        translationUpdate(scenePos, event->position());
        settings_.highlightedVertex = -1;
        settings_.highlightedNormal = -1;
        update();
        return;
    }

    switch (event->buttons()) {
        case Qt::RightButton:
        case Qt::LeftButton: {
            if (settings_.highlightedVertex > -1) {
                setCursor(Qt::ClosedHandCursor);
                // Update position of the control point
                controlCurve_.setVertexPosition(settings_.highlightedVertex, scenePos);
                Matrix3DD selectedConic = getConicAtIndex(settings_.highlightedVertex).getMatrix();
                conicR_.updateBuffers(selectedConic);
                recalculateCurve();
            }
            if (settings_.highlightedNormal > -1) {
                setCursor(Qt::ClosedHandCursor);
                // Update position of the control normal
                controlCurve_.redirectNormalToPoint(settings_.highlightedNormal, scenePos);
                Matrix3DD selectedConic = getConicAtIndex(settings_.highlightedNormal).getMatrix();
                conicR_.updateBuffers(selectedConic);
                recalculateCurve();
            }
            break;
        }
        case Qt::MiddleButton: {
            translationUpdate(scenePos, event->position());
            break;
        }
        default: {
            if (!attemptVertexHighlight(scenePos)) {
                attemptNormalHighlight(scenePos);
            }
            updateCursor(event->modifiers());
            update();
        }
    }
}

/**
 * @brief MainView::keyPressEvent Handles key press events. Used for keyboard
 * shortcuts. Note that, due to the way the UI works, the settings seen in the
 * UI might not be updated when a key is pressed.
 * @param event Key press event.
 */
void MainView::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    const float movementSpeed = 0.02;
    // Only works when the widget has focus!
    switch (event->key()) {
        case Qt::Key_Up:
            controlCurve_.translate({0, movementSpeed});
            recalculateCurve();
            break;
        case Qt::Key_Down:
            controlCurve_.translate({0, -movementSpeed});
            recalculateCurve();
            break;
        case Qt::Key_Left:
            controlCurve_.translate({-movementSpeed, 0});
            recalculateCurve();
            break;
        case Qt::Key_Right:
            controlCurve_.translate({movementSpeed, 0});
            recalculateCurve();
            break;
        case Qt::Key_Shift:
            break;
        case Qt::Key_Control:
            break;
        case Qt::Key_R:
            resetViewMatrix();
            break;
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_X:
            if (settings_.highlightedVertex > -1) {
                // Remove selected control point
                controlCurve_.removePoint(settings_.highlightedVertex);
                settings_.highlightedVertex = -1;
                recalculateCurve();
            }
            break;
    }
    updateCursor(event->modifiers());
}

void MainView::keyReleaseEvent(QKeyEvent *event) {
    QWidget::keyReleaseEvent(event);
    switch (event->key()) {
        case Qt::Key_Shift:
            dragging_ = false;
            break;
        case Qt::Key_Control:
            break;
    }
    updateCursor(event->modifiers());
}

void MainView::mouseDoubleClickEvent(QMouseEvent *event) {
    QWidget::mouseDoubleClickEvent(event);
    if (settings_.highlightedNormal < 0) {
        return;
    }
    controlCurve_.recalculateNormal(settings_.highlightedNormal);
    recalculateCurve();
}

void MainView::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
    float zoom = event->angleDelta().y() > 0 ? settings_.zoomStrength
                                             : 1.0f / settings_.zoomStrength;
    Vector2DD mouseNDC = toNormalizedScreenCoordinates(event->position().x(),
                                                       event->position().y());
    settings_.viewMatrix.translate(mouseNDC.x(), mouseNDC.y());
    settings_.viewMatrix.scale(zoom);
    settings_.viewMatrix.translate(-mouseNDC.x(), -mouseNDC.y());
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    update();
}

void MainView::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    dragging_ = false;
    updateCursor(event->modifiers());
}

void MainView::updateCursor(const Qt::KeyboardModifiers &flags) {
    switch (flags) {
        case Qt::ShiftModifier:
            if (dragging_) {
                setCursor(Qt::ClosedHandCursor);
            } else {
                setCursor(Qt::OpenHandCursor);
            }
            break;
        case Qt::ControlModifier:
            setCursor(Qt::CrossCursor);
            break;
        default:
            if (settings_.highlightedVertex > -1 || settings_.highlightedNormal > -1) {
                setCursor(Qt::OpenHandCursor);
            } else {
                unsetCursor();
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
    if (message.severity() == QOpenGLDebugMessage::LowSeverity ||
        message.type() == QOpenGLDebugMessage::OtherType) {
        return;
    }

    // Format based on source
#define CASE(c)                                                                                    \
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

    if (log)
        qDebug() << "  → Log:" << message;
}

void MainView::setControlCurve(Curve controlCurve) {
    controlCurve_ = controlCurve;
    selectedConicIdx_ = -1;
}

void MainView::recalculateNormals() {
    controlCurve_.recalculateNormals();
    recalculateCurve();
}

void MainView::refineNormals() {
    // controlCurve_.refineNormals(settings_.maxRefinementIterations);
    recalculateCurve();
}

void MainView::refineSelectedNormal() {
    // TODO
    // subCurve_->refineSelectedNormal(settings_.maxRefinementIterations);
    recalculateCurve();
}

bool MainView::saveCurve(const char *fileName, const Curve &curve) {
    std::ofstream file;
    file.open(fileName);

    int prec = 16;

    if (!file.is_open()) {
        QMessageBox msgBox;
        msgBox.setText("Could not open file!");
        msgBox.exec();
        return false;
    } else {
        //        file << "Curve data: x and y coordinates only" << std::endl;

        std::vector<Vector2DD> coords;
        coords = curve.getCoords();

        for (int i = 0; i < coords.size(); i++) {
            file << std::fixed << std::setprecision(prec) << coords[i].x() << " " << std::fixed
                 << std::setprecision(prec) << coords[i].y() << std::endl;
        }
        if (curve.isClosed()) {
            file << std::fixed << std::setprecision(prec) << coords[0].x() << " " << std::fixed
                 << std::setprecision(prec) << coords[0].y() << std::endl;
        }
    }
    file.close();
    return true;
}

bool MainView::saveCurveWithNormals(const char *fileName, const Curve &curve) {
    std::ofstream file;
    file.open(fileName);

    int prec = 16;

    if (!file.is_open()) {
        QMessageBox msgBox;
        msgBox.setText("Could not open file!");
        msgBox.exec();
        return false;
    } else {
        std::vector<Vector2DD> coords;
        std::vector<Vector2DD> normals;
        coords = curve.getCoords();
        normals = curve.getNormals();

        for (int i = 0; i < coords.size(); i++) {
            file << "v " << std::fixed << std::setprecision(prec) << coords[i].x() << " "
                 << std::fixed << std::setprecision(prec) << coords[i].y() << std::endl;
        }
        for (int i = 0; i < normals.size(); i++) {
            file << "vn " << std::fixed << std::setprecision(prec) << normals[i].x() << " "
                 << std::fixed << std::setprecision(prec) << normals[i].y() << std::endl;
        }
    }
    file.close();
    return true;
}
