#include "mainwindow.hpp"

#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QFileDialog>
#include <QOpenGLPaintDevice>
#include <QPushButton>
#include <QSlider>
#include <QMessageBox>

#include "src/core/conics/conicpresets.hpp"
#include "ui/stylepresets.hpp"
#include "core/settings.hpp"
#include "util/imgresourcereader.hpp"

#include "mainview.hpp"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    mainView_ = new MainView(settings_, this);
    presets_ = new conics::ConicPresets(settings_);

    auto *dock = initSideMenu();
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    setMenuBar(initMenuBar());
    setCentralWidget(mainView_);

    conics::ui::applyStylePreset(settings_, conics::ui::getLightModePalette());
    resetView(false);
    mainView_->setFocus();
}

void MainWindow::resetView(bool recalculate) {
    presetName = "Blank";
    mainView_->setSubCurve(std::make_shared<SubdivisionCurve>(presets_->getPreset(presetName)));
    presetLabel->setText(QString("<b>Preset:</b> %1").arg(presetName));
    if (recalculate) {
        mainView_->recalculateCurve();
    }
    subdivStepsSpinBox->setValue(0);
}


MainWindow::~MainWindow() {
}


QDockWidget *MainWindow::initSideMenu() {
    QWidget *sideMenu = new QWidget(this);
    auto *vertLayout = new QVBoxLayout(sideMenu);

    presetLabel = new QLabel("Preset: ");
    vertLayout->addWidget(presetLabel);
    vertLayout->addStretch();

    auto *recalcButton = new QPushButton("Reset Normals");
    connect(recalcButton, &QPushButton::pressed, this, [this] {
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(recalcButton);
    auto *resetPresetButton = new QPushButton("Reset Preset");
    connect(resetPresetButton, &QPushButton::pressed, this, [this] {
        // TODO: extract this
        mainView_->setSubCurve(std::make_shared<SubdivisionCurve>(presets_->getPreset(presetName)));
        presetLabel->setText(QString("<b>Preset:</b> %1").arg(presetName));
        subdivStepsSpinBox->setValue(0);
        closedCurveAction->setChecked(mainView_->getSubCurve()->isClosed());
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(resetPresetButton);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Subdivision Steps"));
    subdivStepsSpinBox = new QSpinBox();
    subdivStepsSpinBox->setMinimum(0);
    subdivStepsSpinBox->setMaximum(20);
    connect(subdivStepsSpinBox, &QSpinBox::valueChanged, [this](int numSteps) {
        mainView_->subdivideCurve(numSteps);
        mainView_->updateBuffers();
    });
    vertLayout->addWidget(subdivStepsSpinBox);
    auto *applySubdivButton = new QPushButton("Apply Subdivision");
    applySubdivButton->setToolTip(
            "<html><head/><body><p>If pressed, applies the subdivision.</body></html>"
    );
    connect(applySubdivButton, &QPushButton::pressed, [this] {
        mainView_->getSubCurve()->applySubdivision();
        mainView_->recalculateCurve();
        subdivStepsSpinBox->setValue(0);
    });
    vertLayout->addWidget(applySubdivButton);


    auto *normSolveCheckBox = new QCheckBox("Normalized Solve");
    normSolveCheckBox->setToolTip(
            "<html><head/><body><p>Enabling this will try to fit a conic using the unit normal constraint. If this option is disabled, a separated scaling value for each normal is calculated.</p></body></html>"
    );
    normSolveCheckBox->setChecked(settings_.normalizedSolve);
    connect(normSolveCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.normalizedSolve = toggled;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(normSolveCheckBox);

    vertLayout->addStretch();
    auto *splitConvexityCheckBox = new QCheckBox("Split Convexity");
    splitConvexityCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, automatically inserts knots before subdividing.</body></html>"
    );
    splitConvexityCheckBox->setChecked(settings_.convexitySplit);
    connect(splitConvexityCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.convexitySplit = toggled;
        mainView_->recalculateCurve();
    });

    auto *insertKnotsButton = new QPushButton("Insert Knots");
    insertKnotsButton->setToolTip(
            "<html><head/><body><p>If pressed, inserts knot points to improve convexity properties.</body></html>"
    );
    connect(insertKnotsButton, &QPushButton::pressed, [this] {
        mainView_->getSubCurve()->insertKnots();
        mainView_->recalculateCurve();
    });

    auto *gravitateAnglesCheckBox = new QCheckBox("Small Angle Bias");
    gravitateAnglesCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, the weighted knot locations gravitate towards the vertex having the smallest angle (i.e. the sharpest spike). If disabled, lets the weighted knot points gravitate towards the vertex having the largest angle.</body></html>"
    );
    gravitateAnglesCheckBox->setChecked(settings_.gravitateSmallerAngles);
    connect(gravitateAnglesCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.gravitateSmallerAngles = toggled;
        mainView_->recalculateCurve();
    });

    auto *weightedKnotLocation = new QCheckBox("Weighted Knot Location");
    weightedKnotLocation->setToolTip(
            "<html><head/><body><p>If enabled, does not insert the knots in the midpoint of each edge, but instead lets the position depend on the ratio between the the two outgoing edges.</body></html>"
    );
    weightedKnotLocation->setChecked(settings_.weightedKnotLocation);
    connect(weightedKnotLocation, &QCheckBox::toggled, [this](bool toggled) {
        settings_.weightedKnotLocation = toggled;
        mainView_->recalculateCurve();
    });


    auto* tensionLabel = new QLabel(QString("Knot Tension: %1").arg(settings_.knotTension));
    auto *tensionSlider = new QSlider(Qt::Horizontal);
    tensionSlider->setToolTip(
            "<html><head/><body><p>Changes how much the normals of newly inserted knot points gravitate to the normal orthogonal to the edge.</p></body></html>"
    );
    tensionSlider->setValue(settings_.knotTension * 100);
    connect(tensionSlider, &QSlider::valueChanged, [this, tensionLabel](int value) {
        settings_.knotTension = value / 100.0f;
        tensionLabel->setText(QString("Knot Tension: %1").arg(settings_.knotTension));
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(splitConvexityCheckBox);
    vertLayout->addWidget(weightedKnotLocation);
    vertLayout->addWidget(gravitateAnglesCheckBox);
    vertLayout->addWidget(tensionLabel);
    vertLayout->addWidget(tensionSlider);
    vertLayout->addWidget(insertKnotsButton);

    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Vertex weights"));
    auto *edgeVertWeightSpinBox = new QDoubleSpinBox();
    edgeVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the points at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    edgeVertWeightSpinBox->setMinimum(0);
    edgeVertWeightSpinBox->setMaximum(maxWeight);
    edgeVertWeightSpinBox->setValue(settings_.pointWeight);
    connect(edgeVertWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings_.pointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeVertWeightSpinBox);

    auto *midVertWeightSpinBox = new QDoubleSpinBox();
    midVertWeightSpinBox->setMinimum(0);
    midVertWeightSpinBox->setMaximum(maxWeight);
    midVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the points at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    midVertWeightSpinBox->setValue(settings_.middlePointWeight);
    connect(midVertWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings_.middlePointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midVertWeightSpinBox);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Normal weights"));
    auto *edgeNormWeightSpinBox = new QDoubleSpinBox();
    edgeNormWeightSpinBox->setMinimum(0);
    edgeNormWeightSpinBox->setMaximum(maxWeight);
    edgeNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the normals at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>"
    );
    edgeNormWeightSpinBox->setValue(settings_.normalWeight);
    connect(edgeNormWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings_.normalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeNormWeightSpinBox);

    auto *midNormWeightSpinBox = new QDoubleSpinBox();
    midNormWeightSpinBox->setMinimum(0);
    midNormWeightSpinBox->setMaximum(maxWeight);
    midNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the normals at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    midNormWeightSpinBox->setValue(settings_.middleNormalWeight);
    connect(midNormWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings_.middleNormalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midNormWeightSpinBox);
    vertLayout->addStretch();

    auto *circleNormsCheckBox = new QCheckBox("Circle Normals");
    circleNormsCheckBox->setToolTip(
            "<html><head/><body><p>Estimate the normals using oscilating circles.</p></body></html>"
    );
    circleNormsCheckBox->setChecked(settings_.circleNormals);
    connect(circleNormsCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.circleNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(circleNormsCheckBox);

    auto *lengthWeightedCheckBox = new QCheckBox("Length Weighted Normals");
    lengthWeightedCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals by taking into consideration the edge lengths.</body></html>"
    );
    lengthWeightedCheckBox->setChecked(settings_.areaWeightedNormals);
    connect(lengthWeightedCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.areaWeightedNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(lengthWeightedCheckBox);

    auto *recalcNormsCheckBox = new QCheckBox("Recalculate Normals");
    recalcNormsCheckBox->setToolTip(
            "<html><head/><body><p>If this option is enabled, the normals will be re-evaluated at every step. If this option is disabled, the vertex points will keep their normals. Any new edge points will obtain the normal of the conic they were sampled from.</p></body></html>"
    );
    recalcNormsCheckBox->setChecked(settings_.recalculateNormals);
    connect(recalcNormsCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.recalculateNormals = toggled;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(recalcNormsCheckBox);

    auto *edgeSampleCheckBox = new QCheckBox("Edge Normal");
    edgeSampleCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, uses a ray perpendicular to the edge to intersect with the conic and sample the new point from. If disabled, uses the average of the edge point normals.</p><p><br/></p><p>In both cases, the ray originates from the middle of the edge.</p></body></html>"
    );
    edgeSampleCheckBox->setChecked(settings_.edgeTangentSample);
    connect(edgeSampleCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.edgeTangentSample = toggled;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeSampleCheckBox);

    vertLayout->addStretch();
    auto *dockWidget = new QDockWidget(this);
    dockWidget->setWidget(sideMenu);
    dockWidget->setFeatures(
            dockWidget->features() & ~(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable));
    return dockWidget;
}


QMenuBar *MainWindow::initMenuBar() {
    auto *menuBar = new QMenuBar();
    menuBar->addMenu(getFileMenu());
    menuBar->addMenu(getPresetMenu());
    menuBar->addMenu(getRenderMenu());

    auto *lightModeToggle = new QAction(menuBar);
    lightModeToggle->setIcon(
            QIcon(util::ImgResourceReader::getPixMap(":/icons/theme.png", {42, 42}, QColor(128, 128, 128))));
    lightModeToggle->setCheckable(true);
    lightModeToggle->setChecked(true); // default is light mode
    connect(lightModeToggle, &QAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            conics::ui::applyStylePreset(settings_, conics::ui::getLightModePalette());
        } else {
            conics::ui::applyStylePreset(settings_, conics::ui::getDarkModePalette());
        }
    });
    auto *rightBar = new QMenuBar(menuBar);
    rightBar->addAction(lightModeToggle);
    menuBar->setCornerWidget(rightBar);
    return menuBar;
}

QMenu *MainWindow::getFileMenu() {
    auto *fileMenu = new QMenu("File");

    auto *newAction = new QAction(QStringLiteral("New"), fileMenu);
    newAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(newAction, &QAction::triggered, [this]() {
        resetView();
    });
    fileMenu->addAction(newAction);


    auto *saveAction = new QAction(QStringLiteral("Save"), fileMenu);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(
                nullptr, "Save Image", "../images/",
                tr("Img Files (*.png *.jpg *.jpeg *.tiff *.tif *pgm *ppm)"));
        if (filePath != "") {
            QPixmap pixmap(mainView_->size());
            mainView_->render(&pixmap);
            bool success = pixmap.toImage().save(filePath);
            if (success) {
                QMessageBox::information(this, "Image Saved", filePath);
                return;
            } else {
                QMessageBox::warning(this, "Failed to save image", "Ensure you provided a file extension:\n:" + filePath);
                return;
            }
        }
        QMessageBox::warning(this, "Failed to save image", "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveAction);

    auto *quitAction = new QAction(QStringLiteral("Quit"), fileMenu);
    quitAction->setShortcutContext(Qt::ApplicationShortcut);
    quitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
    connect(quitAction, &QAction::triggered, []() {
        exit(EXIT_SUCCESS);
    });
    fileMenu->addAction(quitAction);
    return fileMenu;
}

QMenu *MainWindow::getPresetMenu() {
    auto *presetMenu = new QMenu("Presets");

    int i = 1;
    for (auto &name: presets_->getPresetNames()) {
        auto *newAction = new QAction(name, presetMenu);
        newAction->setShortcut(QKeySequence::fromString(QString("Ctrl+%1").arg(i)));
        connect(newAction, &QAction::triggered, [this, name]() {
            presetName = name;
            mainView_->setSubCurve(std::make_shared<SubdivisionCurve>(presets_->getPreset(name)));
            presetLabel->setText(QString("<b>Preset:</b> %1").arg(name));
            subdivStepsSpinBox->setValue(0);
            closedCurveAction->setChecked(mainView_->getSubCurve()->isClosed());
            mainView_->recalculateCurve();
        });
        presetMenu->addAction(newAction);
        i++;
    }

    return presetMenu;
}

QMenu *MainWindow::getRenderMenu() {
    auto *renderMenu = new QMenu("View");

    auto *controlPointsAction = new QAction(QStringLiteral("Control Points"), renderMenu);
    controlPointsAction->setCheckable(true);
    controlPointsAction->setChecked(settings_.showControlPoints);
    controlPointsAction->setShortcut(QKeySequence(Qt::Key_P));
    connect(controlPointsAction, &QAction::triggered, [this](bool toggled) {
        settings_.showControlPoints = toggled;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(controlPointsAction);

    auto *controlCurveAction = new QAction(QStringLiteral("Control Curve"), renderMenu);
    controlCurveAction->setCheckable(true);
    controlCurveAction->setChecked(settings_.showControlCurve);
    controlCurveAction->setShortcut(QKeySequence(Qt::Key_O));
    connect(controlCurveAction, &QAction::triggered, [this](bool toggled) {
        settings_.showControlCurve = toggled;
        mainView_->recalculateCurve();
    });
    renderMenu->addAction(controlCurveAction);

    renderMenu->addSeparator();

    closedCurveAction = new QAction(QStringLiteral("Closed Curve"), renderMenu);
    closedCurveAction->setCheckable(true);
    closedCurveAction->setShortcut(QKeySequence(Qt::Key_C));
    connect(closedCurveAction, &QAction::triggered, [this](bool toggled) {
        if (mainView_->getSubCurve() != nullptr) {
            mainView_->getSubCurve()->setClosed(toggled);
            mainView_->updateBuffers();
        }
    });
    renderMenu->addAction(closedCurveAction);

    renderMenu->addSeparator();

    auto *visualizeStabilityAction = new QAction(QStringLiteral("Visualize Stability"), renderMenu);
    visualizeStabilityAction->setCheckable(true);
    visualizeStabilityAction->setChecked(settings_.visualizeStability);
    visualizeStabilityAction->setShortcut(QKeySequence(Qt::Key_V));
    connect(visualizeStabilityAction, &QAction::triggered, [this](bool toggled) {
        settings_.visualizeStability = toggled;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(visualizeStabilityAction);

    auto *visualizeCurvatureAction = new QAction(QStringLiteral("Visualize Curvature"), renderMenu);
    visualizeCurvatureAction->setCheckable(true);
    visualizeCurvatureAction->setChecked(settings_.visualizeCurvature);
    visualizeCurvatureAction->setShortcut(QKeySequence(Qt::Key_B));
    connect(visualizeCurvatureAction, &QAction::triggered, [this](bool toggled) {
        settings_.visualizeCurvature = toggled;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(visualizeCurvatureAction);

    auto *visualizeNormalsAction = new QAction(QStringLiteral("Visualize Normals"), renderMenu);
    visualizeNormalsAction->setCheckable(true);
    visualizeNormalsAction->setChecked(settings_.visualizeNormals);
    visualizeNormalsAction->setShortcut(QKeySequence(Qt::Key_N));
    connect(visualizeNormalsAction, &QAction::triggered, [this](bool toggled) {
        settings_.visualizeNormals = toggled;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(visualizeNormalsAction);

    auto *normalHandlesAction = new QAction(QStringLiteral("Normal Handles"), renderMenu);
    normalHandlesAction->setCheckable(true);
    normalHandlesAction->setChecked(settings_.normalHandles);
    normalHandlesAction->setShortcut(QKeySequence(Qt::Key_M));
    connect(normalHandlesAction, &QAction::triggered, [this](bool toggled) {
        settings_.normalHandles = toggled;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(normalHandlesAction);

    auto *flipNormals = new QAction(QStringLiteral("Flip Normals"), renderMenu);
    flipNormals->setShortcut(QKeySequence(Qt::ALT | Qt::Key_N));
    connect(flipNormals, &QAction::triggered, [this]() {
        settings_.curvatureSign *= -1;
        mainView_->updateBuffers();
    });
    renderMenu->addAction(flipNormals);
    return renderMenu;
}
