#include "sceneview.hpp"

#include <Eigen/Core>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLVersionFunctionsFactory>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

#include "conis/core/vector.hpp"

namespace conis::gui {

using namespace conis::core;

SceneView::SceneView(ViewSettings &settings, ConisCurve &conisCurve, QWidget *parent)
    : QOpenGLWidget(parent),
      conisCurve_(conisCurve),
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
    conisCurve_.addListener(this);
}

SceneView::~SceneView() {
    debugLogger_->stopLogging();
    conisCurve_.removeListener(this);
}

void SceneView::initializeGL() {
    initializeOpenGLFunctions();
    debugLogger_ = std::make_unique<QOpenGLDebugLogger>();
    connect(debugLogger_.get(),
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
    auto *functions = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>(this->context());
    cnr_.init(functions);
    cr_.init(functions);
    conicR_.init(functions);
    updateBuffers();
}

void SceneView::resizeGL(const int width, const int height) {
    QMatrix4x4 projectMatrix;
    const float halfWidth = width / 2.0f;
    const float halfHeight = height / 2.0f;
    projectMatrix.ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, 0, 1);
    settings_.projectionMatrix = projectMatrix;
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
}

void SceneView::paintGL() {
    const QColor bCol = settings_.style.backgroundCol;
    glClearColor(bCol.redF(), bCol.greenF(), bCol.blueF(), 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cr_.draw();
    cnr_.draw();
    if (settings_.drawSelectedConic) {
        conicR_.draw();
    }
}

void SceneView::viewToFit() {
    const auto &c = conisCurve_.getControlCurve();
    const std::vector<Vector2DD> &verts = c.getVertices();
    if (verts.empty()) {
        return; // Nothing to fit
    }

    // Bounding box
    real_t minX = verts[0].x();
    real_t maxX = verts[0].x();
    real_t minY = verts[0].y();
    real_t maxY = verts[0].y();
    for (const auto &point: verts) {
        minX = std::min(minX, point.x());
        maxX = std::max(maxX, point.x());
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }

    const real_t centerX = (minX + maxX) / 2.0;
    const real_t centerY = (minY + maxY) / 2.0;
    const real_t bwidth = maxX - minX;
    const real_t bheight = maxY - minY;

    const real_t scaleX = (width() / bwidth);
    const real_t scaleY = (height() / bheight);
    const real_t scale = std::min(scaleX, scaleY);

    // Set the view matrix to center the curve and fit it within the viewport
    settings_.viewMatrix.setToIdentity();
    settings_.viewMatrix.scale(scale, scale);
    settings_.viewMatrix.translate(-centerX, -centerY);
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    update();
}

void SceneView::resetViewMatrix() {
    settings_.viewMatrix.setToIdentity();
    settings_.viewMatrix.scale(settings_.initialScale);
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    update();
}

void SceneView::updateBuffers() {
    cr_.updateBuffers(conisCurve_.getSubdivCurve());
    cnr_.updateBuffers(conisCurve_.getControlCurve());
    repaint();
}

void SceneView::onListenerUpdated() {
    updateBuffers();
}

// ===============================================================
//                          Controller parts
// ===============================================================

/**
 * @brief SceneView::toNormalizedScreenCoordinates Normalizes the mouse
 * coordinates to screen coordinates.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @return A vector containing the normalized x and y screen coordinates.
 */
Vector2DD SceneView::toNormalizedScreenCoordinates(const double x, const double y) const {
    const double xRatio = (x - 0) / width();
    const double yRatio = (y - 0) / height();

    const double xScene = xRatio * 2.0f - 1.0f;
    const double yScene = 1.0f - yRatio * 2.0f;
    const QVector4D from = toWorldCoordsMatrix_ * QVector4D(xScene, yScene, 0, 1.0f);
    return { from.x(), from.y() };
}

bool SceneView::attemptVertexHighlight(const Vector2DD &scenePos) {
    float maxDist = settings_.selectRadius;
    if (settings_.highlightedVertex != -1) {
        // Smaller click radius for easy de-selection of points
        maxDist = settings_.deselectRadius;
    }
    // Select control point
    settings_.highlightedVertex = conisCurve_.getControlCurve().findClosestVertex(scenePos, maxDist);
    if (settings_.highlightedVertex > -1) {
        highlightVertex(settings_.highlightedVertex);
        return true;
    }
    return false;
}

bool SceneView::attemptNormalHighlight(const Vector2DD &scenePos) {
    float maxDist = settings_.selectRadius;
    if (settings_.highlightedNormal != -1) {
        // Smaller click radius for easy de-selection of normals
        maxDist = settings_.deselectRadius;
    }
    // Select control point
    settings_.highlightedNormal = conisCurve_.getControlCurve().findClosestNormal(scenePos,
                                                                                  maxDist,
                                                                                  settings_.normalLength);
    if (settings_.highlightedNormal > -1) {
        highlightNormal(settings_.highlightedNormal);
        return true;
    }
    return false;
}

bool SceneView::attemptEdgeHighlight(const Vector2DD &scenePos) {
    float maxDist = settings_.selectRadius;
    if (settings_.highlightedEdge != -1) {
        // Smaller click radius for easy de-selection of edges
        maxDist = settings_.deselectRadius;
    }
    // Select edge
    settings_.highlightedEdge = conisCurve_.getControlCurve().findClosestEdge(scenePos, maxDist);
    if (settings_.highlightedEdge > -1) {
        highlightEdge(settings_.highlightedEdge);
        return true;
    }
    return false;
}

void smoothnessPenalty(const Curve &curve,
                       const int idx,
                       const int testSubdivLevel,
                       const CurvatureType curvatureType) {
    const int n = curve.numPoints();
    if (idx < 0 || idx >= n) {
        return;
    }
    const int i = idx * std::pow(2, testSubdivLevel);
    const real_t curvature_1 = curve.curvatureAtIdx(curve.getPrevIdx(i), curvatureType);
    const real_t curvature1 = curve.curvatureAtIdx(curve.getNextIdx(i), curvatureType);
    real_t p = std::abs(curvature_1 - curvature1);
    if (curvature1 > curvature_1) {
        p = curvature1 / curvature_1;
    } else {
        p = curvature_1 / curvature1;
    }
    #ifdef DEBUG
    std::cout << "Line segment: A-B-C -- " << curve.getPrevIdx(i) << " " << i << " " << curve.getNextIdx(i)
              << std::endl;
    std::cout << "\tCurvature at A: " << curvature_1 << std::endl << "\tCurvature at C: " << curvature1 << std::endl;
    std::cout << "\tPenalty   at B: " << p << std::endl;
    #endif
}

void SceneView::unHighlightAll() {
    settings_.highlightedEdge = -1;
    settings_.highlightedVertex = -1;
    settings_.highlightedNormal = -1;
}
void SceneView::highlightNormal(const int idx) {
    settings_.highlightedEdge = -1;
    settings_.highlightedVertex = -1;
    settings_.highlightedNormal = idx;
}
void SceneView::highlightVertex(const int idx) {
    settings_.highlightedEdge = -1;
    settings_.highlightedVertex = idx;
    settings_.highlightedNormal = -1;
}
void SceneView::highlightEdge(const int idx) {
    settings_.highlightedEdge = idx;
    settings_.highlightedVertex = -1;
    settings_.highlightedNormal = -1;
}
void SceneView::selectVertex(const int idx) {
    settings_.selectedVertex = idx;
#ifdef DEBUG
    smoothnessPenalty(conisCurve_.getSubdivCurve(),
                      settings_.highlightedVertex,
                      conisCurve_.getSubdivLevel(),
                      settings_.curvatureType);
    if (idx > 0) {
        std::cout << "vertex dir: " << conisCurve_.getControlCurve().vertexPointingDir(idx) << std::endl;
    }
#endif
    updateSelectedConic();
    update();
}
void SceneView::selectEdge(const int idx) {
    settings_.selectedEdge = idx;
    if (idx >= 0) {
        std::cout << "edge: " << conisCurve_.getControlCurve().edgePointingDir(idx) << std::endl;
    }
    updateSelectedConic();
    update();
}

void SceneView::updateSelectedConic() {
    if (settings_.selectedEdge < 0) {
        conicR_.updateBuffers(Matrix3DD());
        return;
    }
    const Matrix3DD selectedConic = conisCurve_.getConicAtIndex(settings_.selectedEdge).getMatrix();
    conicR_.updateBuffers(selectedConic);
}

void SceneView::translationUpdate(const Vector2DD &scenePos, const QPointF &mousePos) {
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
void SceneView::mousePressEvent(QMouseEvent *event) {
    // In order to allow keyPressEvents:
    setFocus();
    const Vector2DD scenePos = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
    switch (event->buttons()) {
        case Qt::LeftButton: {
            // Add a point
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                int idx = conisCurve_.addPoint(scenePos);
                highlightVertex(idx);
                selectVertex(idx);
                break;
            }
            // Select a vertex
            if (attemptVertexHighlight(scenePos)) {
                selectVertex(settings_.highlightedVertex);
                break;
            }
            // Failed to select a vertex; unselect any remaining ones
            selectVertex(-1);
            // See if we clicked on a normal handle
            if (attemptNormalHighlight(scenePos)) {
                update();
                break;
            }
            // See if we clicked on an edge
            if (attemptEdgeHighlight(scenePos)) {
                selectEdge(settings_.highlightedEdge);
                break;
            }
            selectEdge(-1);
            break;
        }
        case Qt::RightButton: {
            // Add new control point
            const int idx = conisCurve_.addPoint(scenePos);
            highlightVertex(idx);
            selectVertex(idx);
            break;
        }
        case Qt::MiddleButton: {
            setCursor(Qt::ClosedHandCursor);
            break;
        }
    }
}

void SceneView::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    const Vector2DD scenePos = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
    if (event->buttons() == Qt::LeftButton && event->modifiers().testFlag(Qt::ShiftModifier)) {
        setCursor(Qt::ClosedHandCursor);
        translationUpdate(scenePos, event->position());
        unHighlightAll();
        update();
        return;
    }
    switch (event->buttons()) {
        case Qt::RightButton:
        case Qt::LeftButton: {
            if (settings_.selectedVertex > -1) {
                setCursor(Qt::ClosedHandCursor);
                // Update position of the control point
                conisCurve_.setVertexPosition(settings_.selectedVertex, scenePos);
                updateSelectedConic();
            }
            if (settings_.highlightedNormal > -1) {
                setCursor(Qt::ClosedHandCursor);
                // Update position of the control normal
                conisCurve_.redirectNormalToPoint(settings_.highlightedNormal,
                                                  scenePos,
                                                  settings_.constrainNormalMovement);
                updateSelectedConic();
            }
            break;
        }
        case Qt::MiddleButton: {
            translationUpdate(scenePos, event->position());
            break;
        }
        default: {
            if (!attemptVertexHighlight(scenePos)) {
                if (!attemptNormalHighlight(scenePos)) {
                    attemptEdgeHighlight(scenePos);
                }
            }
            updateCursor(event->modifiers());
            update();
        }
    }
}

/**
 * @brief SceneView::keyPressEvent Handles key press events. Used for keyboard
 * shortcuts. Note that, due to the way the UI works, the settings seen in the
 * UI might not be updated when a key is pressed.
 * @param event Key press event.
 */
void SceneView::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    constexpr float movementSpeed = 0.02;
    // Only works when the widget has focus!
    auto &controlCurve = conisCurve_.getControlCurve();
    switch (event->key()) {
        case Qt::Key_Up:
            conisCurve_.translate({0, movementSpeed});
            break;
        case Qt::Key_Down:
            conisCurve_.translate({0, -movementSpeed});
            break;
        case Qt::Key_Left:
            conisCurve_.translate({-movementSpeed, 0});
            break;
        case Qt::Key_Right:
            conisCurve_.translate({movementSpeed, 0});
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
            if (settings_.selectedVertex > -1) {
                // Remove selected control point
                conisCurve_.removePoint(settings_.selectedVertex);
                selectVertex(-1);
            }
            break;
    }
    updateCursor(event->modifiers());
}

void SceneView::keyReleaseEvent(QKeyEvent *event) {
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

void SceneView::mouseDoubleClickEvent(QMouseEvent *event) {
    QWidget::mouseDoubleClickEvent(event);
    if (settings_.highlightedNormal < 0) {
        return;
    }
    conisCurve_.recalculateNormal(settings_.highlightedNormal);
}

void SceneView::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
    float zoom = event->angleDelta().y() > 0 ? settings_.zoomStrength : 1.0f / settings_.zoomStrength;
    Vector2DD mouseNDC = toNormalizedScreenCoordinates(event->position().x(), event->position().y());
    settings_.viewMatrix.translate(mouseNDC.x(), mouseNDC.y());
    settings_.viewMatrix.scale(zoom);
    settings_.viewMatrix.translate(-mouseNDC.x(), -mouseNDC.y());
    toWorldCoordsMatrix_ = (settings_.projectionMatrix * settings_.viewMatrix).inverted();
    update();
}

void SceneView::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
    dragging_ = false;
    updateCursor(event->modifiers());
}

void SceneView::updateCursor(const Qt::KeyboardModifiers &flags) {
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
            unsetCursor();
            break;
    }
}

/**
 * @brief SceneView::onMessageLogged Helper function for logging messages.
 * @param message The message to log.
 */
void SceneView::onMessageLogged(const QOpenGLDebugMessage &message) {
    bool log = false;
    if (message.severity() == QOpenGLDebugMessage::LowSeverity || message.type() == QOpenGLDebugMessage::OtherType) {
        return;
    }

    // Format based on source
#define CASE(c)                                                                                                        \
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

} // namespace conis::gui
