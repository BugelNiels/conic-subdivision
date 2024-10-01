#include "mainwindow.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QVBoxLayout>

#include "gui/stylepresets.hpp"
#include "src/core/conics/conicpresetfactory.hpp"
#include "src/core/subdivision/conicsubdivider.hpp"
#include "util/imgresourcereader.hpp"

#include "mainview.hpp"
#include "util/objcurvereader.hpp"

using DoubleSlider = ValueSliders::DoubleSlider;
using IntSlider = ValueSliders::IntSlider;
using BoundMode = ValueSliders::BoundMode;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Conic Subdivision Test Tool");
    mainView_ = new MainView(settings_, this);

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
    mainView_->setControlCurve(presetFactory_.getPreset(presetName));
    presetLabel->setText(QString("<b>Preset:</b> %1").arg(presetName));
    if (recalculate) {
        mainView_->recalculateCurve();
    }
    subdivStepsSpinBox->setVal(0);
}

MainWindow::~MainWindow() {}

QDockWidget *MainWindow::initSideMenu() {
    QWidget *sideMenu = new QWidget(this);
    auto *vertLayout = new QVBoxLayout(sideMenu);

    presetLabel = new QLabel("Preset: ");
    vertLayout->addWidget(presetLabel);
    auto *resetPresetButton = new QPushButton("Reset Preset");
    connect(resetPresetButton, &QPushButton::pressed, this, [this] {
        mainView_->setControlCurve(presetFactory_.getPreset(presetName));
        presetLabel->setText(QString("<b>Preset:</b> %1").arg(presetName));
        subdivStepsSpinBox->setVal(0);
        closedCurveAction->setChecked(mainView_->getControlCurve().isClosed());
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(resetPresetButton);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Subdivision Steps"));
    subdivStepsSpinBox = new IntSlider("Steps", 0, 0, 8, BoundMode::LOWER_ONLY);
    connect(subdivStepsSpinBox, &IntSlider::valueUpdated, [this](int numSteps) {
        mainView_->subdivideCurve(numSteps);
        mainView_->updateBuffers();
    });
    vertLayout->addWidget(subdivStepsSpinBox);
    auto *applySubdivButton = new QPushButton("Apply Subdivision");
    applySubdivButton->setToolTip(
            "<html><head/><body><p>If pressed, applies the subdivision.</body></html>");
    connect(applySubdivButton, &QPushButton::pressed, [this] {
        // mainView_->getSubCurve()->applySubdivision(); // TODO: just set the subdivision curve as the control curve
        mainView_->recalculateCurve();
        subdivStepsSpinBox->setVal(0);
    });
    vertLayout->addWidget(applySubdivButton);

#ifndef NDEBUG
    auto *gravitateAnglesCheckBox = new QCheckBox("Small Angle Bias");
    gravitateAnglesCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, the weighted inflection point locations gravitate "
            "towards the vertex having the smallest angle (i.e. the sharpest spike). If disabled, "
            "lets the weighted inflection points gravitate towards the vertex having the largest "
            "angle.</body></html>");
    gravitateAnglesCheckBox->setChecked(settings_.gravitateSmallerAngles);
    gravitateAnglesCheckBox->setEnabled(settings_.weightedInflPointLocation);
    connect(gravitateAnglesCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.gravitateSmallerAngles = toggled;
        mainView_->recalculateCurve();
    });

    auto *weightedInflPointLoc = new QCheckBox("Weighted Inflection Points");
    weightedInflPointLoc->setToolTip(
            "<html><head/><body><p>If enabled, does not insert the inflection points in the "
            "midpoint of each edge, but instead lets the position depend on the ratio between the "
            "the two outgoing edges.</body></html>");
    weightedInflPointLoc->setChecked(settings_.weightedInflPointLocation);
    connect(weightedInflPointLoc,
            &QCheckBox::toggled,
            [this, gravitateAnglesCheckBox](bool toggled) {
                settings_.weightedInflPointLocation = toggled;
                gravitateAnglesCheckBox->setEnabled(settings_.weightedInflPointLocation);
                mainView_->recalculateCurve();
            });

    vertLayout->addStretch();
    auto *splitConvexityCheckBox = new QCheckBox("Split Convexity");
    splitConvexityCheckBox->setToolTip("<html><head/><body><p>If enabled, automatically inserts "
                                       "inflection points before subdividing.</body></html>");
    splitConvexityCheckBox->setChecked(settings_.convexitySplit);
    connect(splitConvexityCheckBox,
            &QCheckBox::toggled,
            [this, weightedInflPointLoc, gravitateAnglesCheckBox](bool toggled) {
                settings_.convexitySplit = toggled;
                weightedInflPointLoc->setEnabled(toggled);
                gravitateAnglesCheckBox->setEnabled(toggled && settings_.weightedInflPointLocation);
                mainView_->recalculateCurve();
            });
    vertLayout->addWidget(splitConvexityCheckBox);
    vertLayout->addWidget(weightedInflPointLoc);
    vertLayout->addWidget(gravitateAnglesCheckBox);
#endif

    auto *patchSizeSlider = new IntSlider("Patch Size", 2, 1, 5, BoundMode::UPPER_LOWER);
    patchSizeSlider->setToolTip("<html><head/><body><p>Changes how much the patch should grow in "
                                "either direction of the edge. The total size of the patch will be "
                                "at most 2 times this number.</p></body></html>");
    connect(patchSizeSlider, &IntSlider::valueUpdated, [this](int value) {
        settings_.patchSize = value;
        mainView_->recalculateCurve();
    });

    auto *dynamicPatchSizeCheckBox = new QCheckBox("Dynamic Patch Size");
    dynamicPatchSizeCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, starting at a patch size of 2, it continuously "
            "increases the patch size until a solution is found where all points lie on the same "
            "branch of the conic.</body></html>");
    dynamicPatchSizeCheckBox->setChecked(settings_.dynamicPatchSize);
    connect(dynamicPatchSizeCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.dynamicPatchSize = toggled;
        mainView_->recalculateCurve();
    });

    vertLayout->addStretch();

    vertLayout->addWidget(patchSizeSlider);
    vertLayout->addWidget(dynamicPatchSizeCheckBox);

    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Vertex weights"));
    auto *edgeVertWeightSpinBox = new DoubleSlider("Inner",
                                                   settings_.middlePointWeight,
                                                   0,
                                                   maxWeight);
    //    edgeVertWeightSpinBox->setStepSize(1.0);
    edgeVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; "
            "font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the "
            "points at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; "
            "font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    connect(edgeVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.middlePointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeVertWeightSpinBox);

    auto *midVertWeightSpinBox = new DoubleSlider("Outer",
                                                  settings_.outerPointWeight,
                                                  0,
                                                  maxWeight);
    midVertWeightSpinBox->setStepSize(1.0);
    midVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; "
            "font-weight:600;&quot;>b</span>-c<span style=&quot; "
            "font-weight:600;&quot;>-</span>d-<span style=&quot; "
            "font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the points "
            "at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; "
            "font-weight:600;&quot;>e</span>.</p></body></html>");
    connect(midVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.outerPointWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midVertWeightSpinBox);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Normal weights"));
    auto *edgeNormWeightSpinBox = new DoubleSlider("Inner",
                                                   settings_.middleNormalWeight,
                                                   0,
                                                   maxWeight);
    edgeNormWeightSpinBox->setStepSize(1.0);
    edgeNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; "
            "font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the "
            "normals at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; "
            "font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    connect(edgeNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.middleNormalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(edgeNormWeightSpinBox);

    auto *midNormWeightSpinBox = new DoubleSlider("Outer",
                                                  settings_.outerNormalWeight,
                                                  0,
                                                  maxWeight);
    midNormWeightSpinBox->setStepSize(1.0);
    midNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; "
            "font-weight:600;&quot;>b</span>-c<span style=&quot; "
            "font-weight:600;&quot;>-</span>d-<span style=&quot; "
            "font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the normals "
            "at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; "
            "font-weight:600;&quot;>e</span>.</p></body></html>");
    connect(midNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        settings_.outerNormalWeight = newVal;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(midNormWeightSpinBox);

    vertLayout->addStretch();
#ifndef NDEBUG
    auto *testToggleCheckBox = new QCheckBox("Test Toggle");
    testToggleCheckBox->setChecked(settings_.testToggle);
    connect(testToggleCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings_.testToggle = toggled;
        mainView_->recalculateCurve();
    });
    vertLayout->addWidget(testToggleCheckBox);
    vertLayout->addStretch();
#endif

    auto *recalcButton = new QPushButton("Reset Normals");
    connect(recalcButton, &QPushButton::pressed, this, [this] {
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(recalcButton);

    auto *regularNormalsRadioButton = new QRadioButton("Regular Normals");
    regularNormalsRadioButton->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals using half the angle "
            "between the two adjacent edges.</body></html>");
    regularNormalsRadioButton->setChecked(!settings_.areaWeightedNormals);
    connect(regularNormalsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.areaWeightedNormals = false;
        settings_.circleNormals = false;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(regularNormalsRadioButton);

    auto *lengthWeightedRadioButton = new QRadioButton("Length Weighted Normals");
    lengthWeightedRadioButton->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals by taking into "
            "consideration the edge lengths.</body></html>");
    lengthWeightedRadioButton->setChecked(settings_.areaWeightedNormals);
    connect(lengthWeightedRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.areaWeightedNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(lengthWeightedRadioButton);
#ifndef NDEBUG
    auto *circleNormsRadioButton = new QRadioButton("Circle Normals");
    circleNormsRadioButton->setToolTip("<html><head/><body><p>Estimate the normals using "
                                       "oscilating circles.</p></body></html>");
    circleNormsRadioButton->setChecked(settings_.circleNormals);
    connect(circleNormsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        settings_.circleNormals = toggled;
        mainView_->recalculateNormals();
    });
    vertLayout->addWidget(circleNormsRadioButton);
#endif
    vertLayout->addStretch();

    auto *refineNormalsButton = new QPushButton("Refine Normals");
    connect(refineNormalsButton, &QPushButton::pressed, this, [this] {
        mainView_->refineNormals();
    });
    vertLayout->addWidget(refineNormalsButton);

    auto *refineSelectedNormalButton = new QPushButton("Refine Selected Normal");
    connect(refineSelectedNormalButton, &QPushButton::pressed, this, [this] {
        mainView_->refineSelectedNormal();
    });
    vertLayout->addWidget(refineSelectedNormalButton);

    vertLayout->addWidget(new QLabel("Test Subdiv Level"));
    IntSlider *testSubdivLevelSpinBox = new IntSlider("Test Subdiv Level",
                                                      settings_.testSubdivLevel,
                                                      1,
                                                      7,
                                                      BoundMode::LOWER_ONLY);
    connect(testSubdivLevelSpinBox, &IntSlider::valueUpdated, [this](int subdivLvl) {
        settings_.testSubdivLevel = subdivLvl;
    });
    vertLayout->addWidget(testSubdivLevelSpinBox);

    vertLayout->addWidget(new QLabel("Max iterations"));
    IntSlider *refinementIterationsSpinBox = new IntSlider("Iterations",
                                                           settings_.maxRefinementIterations,
                                                           1,
                                                           100,
                                                           BoundMode::LOWER_ONLY);
    connect(refinementIterationsSpinBox, &IntSlider::valueUpdated, [this](int numSteps) {
        settings_.maxRefinementIterations = numSteps;
    });
    vertLayout->addWidget(refinementIterationsSpinBox);

    vertLayout->addWidget(new QLabel("Angle limit (* 1e-8)"));
    DoubleSlider *angleLimitSpinBox = new DoubleSlider("Angle limit",
                                                       settings_.angleLimit * 1e8,
                                                       1.0e-12,
                                                       1,
                                                       BoundMode::UPPER_LOWER);
    connect(angleLimitSpinBox, &DoubleSlider::valueUpdated, [this](double angleLimit) {
        settings_.angleLimit = angleLimit / 1e8;
    });
    vertLayout->addWidget(angleLimitSpinBox);

    vertLayout->addStretch();

    auto *curvatureScaleSlider = new DoubleSlider("Curvature Scale",
                                                  settings_.curvatureScale,
                                                  0.01,
                                                  10);
    curvatureScaleSlider->setToolTip(
            "<html><head/><body><p>Changes how long the curvature combs are.</p></body></html>");
    connect(curvatureScaleSlider, &DoubleSlider::valueUpdated, [this](double value) {
        settings_.curvatureScale = value;
        mainView_->updateBuffers();
    });
    vertLayout->addWidget(curvatureScaleSlider);
    vertLayout->addStretch();

    auto *dockWidget = new QDockWidget(this);
    dockWidget->setWidget(sideMenu);
    dockWidget->setFeatures(dockWidget->features() &
                            ~(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable));
    return dockWidget;
}

QMenuBar *MainWindow::initMenuBar() {
    auto *menuBar = new QMenuBar();
    menuBar->addMenu(getFileMenu());
    menuBar->addMenu(getPresetMenu());
    menuBar->addMenu(getRenderMenu());
    menuBar->addMenu(getWindowMenu());

    auto *lightModeToggle = new QAction(menuBar);
    lightModeToggle->setIcon(QIcon(util::ImgResourceReader::getPixMap(":/icons/theme.png",
                                                                      {42, 42},
                                                                      QColor(128, 128, 128))));
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
        QString filePath = QFileDialog::getOpenFileName(nullptr,
                                                        "Load Curve",
                                                        "../curves/",
                                                        tr("Txt Files (*.txt)"));
        if (filePath == "") {
            return;
        }
        ObjCurveReader reader;
        Curve curve = reader.loadCurveFromObj(filePath);
        mainView_->setControlCurve(presetFactory_.getPreset(presetName));
        subdivStepsSpinBox->setVal(0);
        closedCurveAction->setChecked(curve.isClosed());
        mainView_->recalculateCurve();
    });
    fileMenu->addAction(openAction);

    auto *saveAction = new QAction(QStringLiteral("Save"), fileMenu);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(
                nullptr,
                "Save Image",
                "../images/",
                tr("Img Files (*.png *.jpg *.jpeg *.tiff *.tif *pgm *ppm)"));
        if (filePath != "") {
            QPixmap pixmap(mainView_->size());
            mainView_->render(&pixmap);
            bool success = pixmap.toImage().save(filePath);
            if (success) {
                QMessageBox::information(this, "Image Saved", filePath);
                return;
            } else {
                QMessageBox::warning(this,
                                     "Failed to save image",
                                     "Ensure you provided a file extension:\n:" + filePath);
                return;
            }
        }
        QMessageBox::warning(this,
                             "Failed to save image",
                             "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveAction);

    // outputs list of points (x y coords) in a text file (for Lucia/Matlab)
    auto *saveCurveAction = new QAction(QStringLiteral("SaveCurve"), fileMenu);
    saveCurveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(saveCurveAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Curve",
                                                        "../curves/",
                                                        tr("TXT File (*.txt)"));
        if (filePath != "") {
            QByteArray bytes = filePath.toStdString().c_str();
            char *file_name;
            file_name = bytes.data();

            bool success = mainView_->saveCurve(file_name, mainView_->getSubdivCurve());

            if (success) {
                QMessageBox::information(this, "Curve Saved", filePath);
                return;
            } else {
                QMessageBox::warning(this,
                                     "Failed to save curve",
                                     "Ensure you provided a file extension:\n:" + filePath);
                return;
            }
        }
        QMessageBox::warning(this,
                             "Failed to save curve",
                             "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveCurveAction);

    // outputs list of points and normals
    auto *saveCurveNAction = new QAction(QStringLiteral("SaveCurveN"), fileMenu);
    //    saveCurveNAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(saveCurveNAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Curve",
                                                        "../curves/",
                                                        tr("Obj File (*.obj)"));
        if (filePath != "") {
            QByteArray bytes = filePath.toStdString().c_str();
            char *file_name;
            file_name = bytes.data();

            bool success = mainView_->saveCurveWithNormals(file_name, mainView_->getSubdivCurve());

            if (success) {
                QMessageBox::information(this, "Curve Saved", filePath);
                return;
            } else {
                QMessageBox::warning(this,
                                     "Failed to save curve",
                                     "Ensure you provided a file extension:\n:" + filePath);
                return;
            }
        }
        QMessageBox::warning(this,
                             "Failed to save curve",
                             "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveCurveNAction);

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
    for (auto &name: presetFactory_.getPresetNames()) {
        auto *newAction = new QAction(name, presetMenu);
        newAction->setShortcut(QKeySequence::fromString(QString("Ctrl+%1").arg(i)));
        connect(newAction, &QAction::triggered, [this, name]() {
            presetName = name;
            mainView_->setControlCurve(presetFactory_.getPreset(presetName));
            presetLabel->setText(QString("<b>Preset:</b> %1").arg(name));
            subdivStepsSpinBox->setVal(0);
            closedCurveAction->setChecked(mainView_->getControlCurve().isClosed());
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
    closedCurveAction->setChecked(true);
    closedCurveAction->setShortcut(QKeySequence(Qt::Key_C));
    connect(closedCurveAction, &QAction::triggered, [this](bool toggled) {
        mainView_->getControlCurve().setClosed(toggled);
        mainView_->updateBuffers();
    });
    renderMenu->addAction(closedCurveAction);

    renderMenu->addSeparator();

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
