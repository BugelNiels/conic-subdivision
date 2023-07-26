#include "mainwindow.hpp"

#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
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
#include <QRadioButton>

#include "src/core/conics/conicpresets.hpp"
#include "gui/stylepresets.hpp"
#include "util/imgresourcereader.hpp"

#include "mainview.hpp"
#include "util/objcurvereader.hpp"

using DoubleSlider = ValueSliders::DoubleSlider;
using IntSlider = ValueSliders::IntSlider;
using BoundMode = ValueSliders::BoundMode;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {
    mainView_ = new MainView(settings_, this);
    presets_ = new conics::ConicPresets(settings_);

    dock_ = initSideMenu();
    addDockWidget(Qt::LeftDockWidgetArea, dock_);
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
    subdivStepsSpinBox = new IntSlider("Steps", 0, 0, 8, BoundMode::UPPER_LOWER);
    connect(subdivStepsSpinBox, &IntSlider::valueUpdated, [this](int numSteps) {
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


    auto *gravitateAnglesCheckBox = new QCheckBox("Small Angle Bias");
    gravitateAnglesCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, the weighted inflection point locations gravitate towards the vertex having the smallest angle (i.e. the sharpest spike). If disabled, lets the weighted inflection points gravitate towards the vertex having the largest angle.</body></html>"
    );
    gravitateAnglesCheckBox->setChecked(settings_.gravitateSmallerAngles);
    gravitateAnglesCheckBox->setEnabled(settings_.weightedKnotLocation);
    connect(gravitateAnglesCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.gravitateSmallerAngles = toggled;
        mainView_->recalculateCurve();
    });

    auto *weightedKnotLocation = new QCheckBox("Weighted Inflection Points");
    weightedKnotLocation->setToolTip(
            "<html><head/><body><p>If enabled, does not insert the inflection points in the midpoint of each edge, but instead lets the position depend on the ratio between the the two outgoing edges.</body></html>"
    );
    weightedKnotLocation->setChecked(settings_.weightedKnotLocation);
    connect(weightedKnotLocation, &QCheckBox::toggled, [this, gravitateAnglesCheckBox](bool toggled) {
        settings_.weightedKnotLocation = toggled;
        gravitateAnglesCheckBox->setEnabled(settings_.weightedKnotLocation);
        mainView_->recalculateCurve();
    });


    auto *tensionSlider = new DoubleSlider("Tension", 0.8, 0, 1, BoundMode::UPPER_LOWER);
    tensionSlider->setToolTip(
            "<html><head/><body><p>Changes how much the normals of newly inserted inflection points gravitate to the normal orthogonal to the edge.</p></body></html>"
    );
    connect(tensionSlider, &DoubleSlider::valueUpdated, [this](double value) {
        settings_.knotTension = value;
        mainView_->recalculateCurve();
    });

    vertLayout->addStretch();
    auto *splitConvexityCheckBox = new QCheckBox("Split Convexity");
    splitConvexityCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, automatically inserts inflection points before subdividing.</body></html>"
    );
    splitConvexityCheckBox->setChecked(settings_.convexitySplit);
    connect(splitConvexityCheckBox, &QCheckBox::toggled,
            [this, weightedKnotLocation, gravitateAnglesCheckBox, tensionSlider](bool toggled) {
                settings_.convexitySplit = toggled;
                weightedKnotLocation->setEnabled(toggled);
                gravitateAnglesCheckBox->setEnabled(toggled && settings_.weightedKnotLocation);
                tensionSlider->setEnabled(toggled);
                mainView_->recalculateCurve();
            });

    auto *insertKnotsButton = new QPushButton("Insert Inflection Points");
    insertKnotsButton->setToolTip(
            "<html><head/><body><p>If pressed, inserts inflection points points to improve convexity properties.</body></html>"
    );
    connect(insertKnotsButton, &QPushButton::pressed, [this] {
        mainView_->getSubCurve()->insertKnots();
        mainView_->recalculateCurve();
    });

    vertLayout->addWidget(splitConvexityCheckBox);
    vertLayout->addWidget(weightedKnotLocation);
    vertLayout->addWidget(gravitateAnglesCheckBox);
    vertLayout->addWidget(tensionSlider);
    vertLayout->addWidget(insertKnotsButton);

    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Vertex weights"));
    auto *edgeVertWeightSpinBox = new DoubleSlider("Inner", settings_.middlePointWeight, 0, maxWeight);
    edgeVertWeightSpinBox->setStepSize(1.0);
    edgeVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the points at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    connect(edgeVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.middlePointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeVertWeightSpinBox);

    auto *midVertWeightSpinBox = new DoubleSlider("Outer", settings_.outerPointWeight, 0, maxWeight);
    midVertWeightSpinBox->setStepSize(1.0);
    midVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the points at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    connect(midVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.outerPointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midVertWeightSpinBox);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Normal weights"));
    auto *edgeNormWeightSpinBox = new DoubleSlider("Inner", settings_.middleNormalWeight, 0, maxWeight);
    edgeNormWeightSpinBox->setStepSize(1.0);
    edgeNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the normals at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>"
    );
    connect(edgeNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.middleNormalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeNormWeightSpinBox);

    auto *midNormWeightSpinBox = new DoubleSlider("Outer", settings_.outerNormalWeight, 0, maxWeight);
    midNormWeightSpinBox->setStepSize(1.0);
    midNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the normals at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    connect(midNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.outerNormalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midNormWeightSpinBox);

    vertLayout->addStretch();
    auto *testToggleCheckBox = new QCheckBox("Test Toggle");
    testToggleCheckBox->setChecked(settings_.testToggle);
    connect(testToggleCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.testToggle = toggled;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(testToggleCheckBox);
    vertLayout->addStretch();

    auto *regularNormalsRadioButton = new QRadioButton("Regular Normals");
    regularNormalsRadioButton->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals using half the angle between the two adjacent edges.</body></html>"
    );
    regularNormalsRadioButton->setChecked(!settings_.areaWeightedNormals);
    connect(regularNormalsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.areaWeightedNormals = false;
        settings_.circleNormals = false;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(regularNormalsRadioButton);

    auto *lengthWeightedRadioButton = new QRadioButton("Length Weighted Normals");
    lengthWeightedRadioButton->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals by taking into consideration the edge lengths.</body></html>"
    );
    lengthWeightedRadioButton->setChecked(settings_.areaWeightedNormals);
    connect(lengthWeightedRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.areaWeightedNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(lengthWeightedRadioButton);

    auto *circleNormsRadioButton = new QRadioButton("Circle Normals");
    circleNormsRadioButton->setToolTip(
            "<html><head/><body><p>Estimate the normals using oscilating circles.</p></body></html>"
    );
    circleNormsRadioButton->setChecked(settings_.circleNormals);
    connect(circleNormsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.circleNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(circleNormsRadioButton);


    vertLayout->addStretch();

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

    auto *curvatureScaleSlider = new DoubleSlider("Curvature Scale", settings_.curvatureScale, 0.01, 2);
    curvatureScaleSlider->setToolTip(
            "<html><head/><body><p>Changes how long the curvature combs are.</p></body></html>"
    );
    connect(curvatureScaleSlider, &DoubleSlider::valueUpdated, [this](double value) {
        settings_.curvatureScale = value;
        mainView_->updateBuffers();
    });
    vertLayout->addWidget(curvatureScaleSlider);
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
    menuBar->addMenu(getWindowMenu());

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

    auto *openAction = new QAction(QStringLiteral("Open"), fileMenu);
    openAction->setShortcutContext(Qt::ApplicationShortcut);
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getOpenFileName(
                nullptr, "Load Curve", "../curves/",
                tr("Obj Files (*.obj)"));
        if (filePath == "") {
            return;
        }
        ObjCurveReader reader(settings_);
        mainView_->setSubCurve(std::make_shared<SubdivisionCurve>(reader.loadCurveFromObj(filePath)));
        // TODO: extract function for new curves
        subdivStepsSpinBox->setValue(0);
        closedCurveAction->setChecked(mainView_->getSubCurve()->isClosed());
        mainView_->recalculateCurve();
    });
    fileMenu->addAction(openAction);

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
                QMessageBox::warning(this, "Failed to save image",
                                     "Ensure you provided a file extension:\n:" + filePath);
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

QMenu *MainWindow::getWindowMenu() {
    auto *windowMenu = new QMenu("Window");

    auto *resizeAction = new QAction(QStringLiteral("Resize"), windowMenu);
    connect(resizeAction, &QAction::triggered, [this](bool toggled) {
        this->showNormal();
        this->resize(dock_->width() + 800, 800);
    });
    windowMenu->addAction(resizeAction);
    return windowMenu;
}
