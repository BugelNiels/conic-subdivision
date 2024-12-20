#include "mainwindow.hpp"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
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

#include "conis/core/curve/curveloader.hpp"
#include "conis/core/curve/curvesaver.hpp"
#include "sceneview.hpp"
#include "stylepresets.hpp"
#include "util/imgresourcereader.hpp"

using DoubleSlider = ValueSliders::DoubleSlider;
using IntSlider = ValueSliders::IntSlider;
using BoundMode = ValueSliders::BoundMode;

namespace conis::gui {

MainWindow::MainWindow(core::ConisCurve &conisCurve,
                       core::SubdivisionSettings &subdivSettings,
                       core::NormalRefinementSettings &normRefSettings,
                       ViewSettings &viewSettings,
                       QWidget *parent)
    : QMainWindow(parent),
      viewSettings_(viewSettings),
      subdivSettings_(subdivSettings),
      normRefSettings_(normRefSettings) {
    setWindowTitle("Conic Subdivision Test Tool");
    sceneView_ = new SceneView(viewSettings, conisCurve, this);

    dock_ = initSideMenu();
    addDockWidget(Qt::LeftDockWidgetArea, dock_);
    setMenuBar(initMenuBar());
    setCentralWidget(sceneView_);

    applyStylePreset(viewSettings, getLightModePalette());
    resetView();
    sceneView_->setFocus();
}

void MainWindow::resetView() {
    presetName = "Blank";
    viewSettings_.selectedVertex = -1;
    sceneView_->getConisCurve().setControlCurve(presetFactory_.getPreset(presetName));
    presetLabel->setText(QString("<b>Preset:</b> %1").arg(QString::fromStdString(presetName)));
    subdivStepsSpinBox->setVal(0);
}

QDockWidget *MainWindow::initSideMenu() {
    QWidget *sideMenu = new QWidget(this);
    auto *vertLayout = new QVBoxLayout(sideMenu);

    presetLabel = new QLabel("Preset: ");
    vertLayout->addWidget(presetLabel);
    auto *resetPresetButton = new QPushButton("Reset Preset");
    connect(resetPresetButton, &QPushButton::pressed, this, [this] {
        auto &conisCurve = sceneView_->getConisCurve();
        subdivStepsSpinBox->setVal(0);
        presetLabel->setText(QString("<b>Preset:</b> %1").arg(QString::fromStdString(presetName)));
        viewSettings_.selectedVertex = -1;
        conisCurve.setControlCurve(presetFactory_.getPreset(presetName));
        closedCurveAction->setChecked(conisCurve.getControlCurve().isClosed());
    });
    vertLayout->addWidget(resetPresetButton);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Subdivision Steps"));
    subdivStepsSpinBox = new IntSlider("Steps", 0, 0, 8, BoundMode::LOWER_ONLY);
    connect(subdivStepsSpinBox, &IntSlider::valueUpdated, [this](int numSteps) {
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.subdivideCurve(numSteps);
    });
    vertLayout->addWidget(subdivStepsSpinBox);
    auto *applySubdivButton = new QPushButton("Apply Subdivision");
    applySubdivButton->setToolTip("<html><head/><body><p>If pressed, applies the subdivision.</body></html>");
    connect(applySubdivButton, &QPushButton::pressed, [this] {
        auto &conisCurve = sceneView_->getConisCurve();
        auto curv = conisCurve.getSubdivCurve();
        viewSettings_.selectedVertex = -1;
        conisCurve.setControlCurve(curv);
        subdivStepsSpinBox->setVal(0);
    });
    vertLayout->addWidget(applySubdivButton);

    auto *insertInflPointsButton = new QPushButton("Insert Inflection Points");
    insertInflPointsButton->setToolTip("<html><head/><body><p>If pressed, inserts inflection points.</body></html>");
    connect(insertInflPointsButton, &QPushButton::pressed, [this] {
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.insertInflectionPoints();
    });
    vertLayout->addWidget(insertInflPointsButton);

#ifndef NDEBUG
    auto *gravitateAnglesCheckBox = new QCheckBox("Small Angle Bias");
    gravitateAnglesCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, the weighted inflection point locations gravitate "
            "towards the vertex having the smallest angle (i.e. the sharpest spike). If disabled, "
            "lets the weighted inflection points gravitate towards the vertex having the largest "
            "angle.</body></html>");
    gravitateAnglesCheckBox->setChecked(subdivSettings_.gravitateSmallerAngles);
    gravitateAnglesCheckBox->setEnabled(subdivSettings_.weightedInflPointLocation);
    connect(gravitateAnglesCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.gravitateSmallerAngles = toggled;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });

    auto *weightedInflPointLoc = new QCheckBox("Weighted Inflection Points");
    weightedInflPointLoc->setToolTip(
            "<html><head/><body><p>If enabled, does not insert the inflection points in the "
            "midpoint of each edge, but instead lets the position depend on the ratio between the "
            "the two outgoing edges.</body></html>");
    weightedInflPointLoc->setChecked(subdivSettings_.weightedInflPointLocation);
    connect(weightedInflPointLoc, &QCheckBox::toggled, [this, gravitateAnglesCheckBox](bool toggled) {
        subdivSettings_.weightedInflPointLocation = toggled;
        gravitateAnglesCheckBox->setEnabled(subdivSettings_.weightedInflPointLocation);
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });

    vertLayout->addStretch();
    auto *splitConvexityCheckBox = new QCheckBox("Split Convexity");
    splitConvexityCheckBox->setToolTip("<html><head/><body><p>If enabled, automatically inserts "
                                       "inflection points before subdividing.</body></html>");
    splitConvexityCheckBox->setChecked(subdivSettings_.convexitySplit);
    connect(splitConvexityCheckBox,
            &QCheckBox::toggled,
            [this, weightedInflPointLoc, gravitateAnglesCheckBox](bool toggled) {
                subdivSettings_.convexitySplit = toggled;
                weightedInflPointLoc->setEnabled(toggled);
                gravitateAnglesCheckBox->setEnabled(toggled && subdivSettings_.weightedInflPointLocation);
                auto &conisCurve = sceneView_->getConisCurve();
                conisCurve.resubdivide();
            });
    vertLayout->addWidget(splitConvexityCheckBox);
    vertLayout->addWidget(weightedInflPointLoc);
    vertLayout->addWidget(gravitateAnglesCheckBox);

    auto *patchSizeSlider = new IntSlider("Patch Size", 2, 1, 5, BoundMode::UPPER_LOWER);
    patchSizeSlider->setToolTip("<html><head/><body><p>Changes how much the patch should grow in "
                                "either direction of the edge. The total size of the patch will be "
                                "at most 2 times this number.</p></body></html>");
    connect(patchSizeSlider, &IntSlider::valueUpdated, [this](int value) {
        subdivSettings_.patchSize = value;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });

    auto *dynamicPatchSizeCheckBox = new QCheckBox("Dynamic Patch Size");
    dynamicPatchSizeCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, starting at a patch size of 2, it continuously "
            "increases the patch size until a solution is found where all points lie on the same "
            "branch of the conic.</body></html>");
    dynamicPatchSizeCheckBox->setChecked(subdivSettings_.dynamicPatchSize);
    connect(dynamicPatchSizeCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.dynamicPatchSize = toggled;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });
    vertLayout->addWidget(patchSizeSlider);
    vertLayout->addWidget(dynamicPatchSizeCheckBox);
#endif

    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Vertex weights"));
    auto *edgeVertWeightSpinBox = new DoubleSlider("Inner", subdivSettings_.middlePointWeight, 0, maxWeight);
    //    edgeVertWeightSpinBox->setStepSize(1.0);
    edgeVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; "
            "font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the "
            "points at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; "
            "font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    connect(edgeVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        subdivSettings_.middlePointWeight = newVal;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });
    vertLayout->addWidget(edgeVertWeightSpinBox);

    auto *midVertWeightSpinBox = new DoubleSlider("Outer", subdivSettings_.outerPointWeight, 0, maxWeight);
    midVertWeightSpinBox->setStepSize(1.0);
    midVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; "
            "font-weight:600;&quot;>b</span>-c<span style=&quot; "
            "font-weight:600;&quot;>-</span>d-<span style=&quot; "
            "font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the points "
            "at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; "
            "font-weight:600;&quot;>e</span>.</p></body></html>");
    connect(midVertWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        subdivSettings_.outerPointWeight = newVal;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });
    vertLayout->addWidget(midVertWeightSpinBox);

    vertLayout->addWidget(new QLabel("Normal weights"));
    auto *edgeNormWeightSpinBox = new DoubleSlider("Inner", subdivSettings_.middleNormalWeight, 0, maxWeight);
    edgeNormWeightSpinBox->setStepSize(1.0);
    edgeNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; "
            "font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the "
            "normals at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; "
            "font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    connect(edgeNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        subdivSettings_.middleNormalWeight = newVal;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });
    vertLayout->addWidget(edgeNormWeightSpinBox);

    auto *midNormWeightSpinBox = new DoubleSlider("Outer", subdivSettings_.outerNormalWeight, 0, maxWeight);
    midNormWeightSpinBox->setStepSize(1.0);
    midNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; "
            "font-weight:600;&quot;>b</span>-c<span style=&quot; "
            "font-weight:600;&quot;>-</span>d-<span style=&quot; "
            "font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the normals "
            "at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; "
            "font-weight:600;&quot;>e</span>.</p></body></html>");
    connect(midNormWeightSpinBox, &DoubleSlider::valueUpdated, [this](double newVal) {
        subdivSettings_.outerNormalWeight = newVal;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.resubdivide();
    });
    vertLayout->addWidget(midNormWeightSpinBox);

    vertLayout->addStretch();
#ifndef NDEBUG
    auto *testToggleCheckBox = new QCheckBox("Test Toggle");
    testToggleCheckBox->setChecked(subdivSettings_.testToggle);
    connect(testToggleCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.testToggle = toggled;
        sceneView_->getConisCurve().resubdivide();
    });
    vertLayout->addWidget(testToggleCheckBox);
    vertLayout->addStretch();
#endif

    auto *recalcButton = new QPushButton("Reset Normals");
    connect(recalcButton, &QPushButton::pressed, this, [this] {
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.recalculateNormals();
    });
    vertLayout->addWidget(recalcButton);

    auto *regularNormalsRadioButton = new QRadioButton("Regular Normals");
    regularNormalsRadioButton->setToolTip(
            "<html><head/><body><p>If enabled, approximates the normals using half the angle "
            "between the two adjacent edges.</body></html>");
    regularNormalsRadioButton->setChecked(!subdivSettings_.areaWeightedNormals);
    connect(regularNormalsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.areaWeightedNormals = false;
        subdivSettings_.circleNormals = false;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.recalculateNormals();
    });
    vertLayout->addWidget(regularNormalsRadioButton);

    auto *lengthWeightedRadioButton = new QRadioButton("Length Weighted Normals");
    lengthWeightedRadioButton->setToolTip("<html><head/><body><p>If enabled, approximates the normals by taking into "
                                          "consideration the edge lengths.</body></html>");
    lengthWeightedRadioButton->setChecked(subdivSettings_.areaWeightedNormals);
    connect(lengthWeightedRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.areaWeightedNormals = toggled;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.recalculateNormals();
    });
    vertLayout->addWidget(lengthWeightedRadioButton);
#ifndef NDEBUG
    auto *circleNormsRadioButton = new QRadioButton("Circle Normals");
    circleNormsRadioButton->setToolTip("<html><head/><body><p>Estimate the normals using "
                                       "oscilating circles.</p></body></html>");
    circleNormsRadioButton->setChecked(subdivSettings_.circleNormals);
    connect(circleNormsRadioButton, &QCheckBox::toggled, [this](bool toggled) {
        subdivSettings_.circleNormals = toggled;
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.recalculateNormals();
    });
    vertLayout->addWidget(circleNormsRadioButton);
#endif
    vertLayout->addStretch();

    // Normal refinement
    // auto *refineNormalsButton = new QPushButton("Refine Normals");
    // connect(refineNormalsButton, &QPushButton::pressed, this, [this] {
    //     setCursor(Qt::WaitCursor);
    //     auto &conisCurve = sceneView_->getConisCurve();
    //     conisCurve.refineNormalsProgressively(viewSettings_.curvatureType);
    //     unsetCursor();
    // });
    // vertLayout->addWidget(refineNormalsButton);
    //
    // auto *refineSelectedNormalButton = new QPushButton("Refine Selected Normal");
    // connect(refineSelectedNormalButton, &QPushButton::pressed, this, [this] {
    //     setCursor(Qt::WaitCursor);
    //     auto &conisCurve = sceneView_->getConisCurve();
    //     conisCurve.refineNormalProgressively(viewSettings_.selectedVertex, viewSettings_.curvatureType);
    //     unsetCursor();
    // });
    // vertLayout->addWidget(refineSelectedNormalButton);
    //
    // vertLayout->addWidget(new QLabel("Test Subdiv Level"));
    // IntSlider *testSubdivLevelSpinBox = new IntSlider("Test Subdiv Level",
    //                                                   normRefSettings_.testSubdivLevel,
    //                                                   1,
    //                                                   7,
    //                                                   BoundMode::LOWER_ONLY);
    // connect(testSubdivLevelSpinBox, &IntSlider::valueUpdated, [this](int subdivLvl) {
    //     normRefSettings_.testSubdivLevel = subdivLvl;
    // });
    // vertLayout->addWidget(testSubdivLevelSpinBox);
    //
    // vertLayout->addWidget(new QLabel("Max iterations"));
    // IntSlider *refinementIterationsSpinBox = new IntSlider("Iterations",
    //                                                        normRefSettings_.maxRefinementIterations,
    //                                                        1,
    //                                                        100,
    //                                                        BoundMode::LOWER_ONLY);
    // connect(refinementIterationsSpinBox, &IntSlider::valueUpdated, [this](int numSteps) {
    //     normRefSettings_.maxRefinementIterations = numSteps;
    // });
    // vertLayout->addWidget(refinementIterationsSpinBox);
    //
    // vertLayout->addWidget(new QLabel("Angle limit (degrees)"));
    // DoubleSlider *angleLimitSpinBox = new DoubleSlider("Angle limit",
    //                                                    normRefSettings_.angleLimit,
    //                                                    0.0000001,
    //                                                    1,
    //                                                    BoundMode::UPPER_LOWER);
    // connect(angleLimitSpinBox, &DoubleSlider::valueUpdated, [this](double angleLimit) {
    //     normRefSettings_.angleLimit = angleLimit * (M_PI / 180.0);
    // });
    // vertLayout->addWidget(angleLimitSpinBox);
    //
    // vertLayout->addStretch();

    auto *curvatureTypeDropdown = new QComboBox();
    curvatureTypeDropdown->addItem("Circle Radius", conis::core::CurvatureType::CIRCLE_RADIUS);
    curvatureTypeDropdown->addItem("Winding Number Theorem", conis::core::CurvatureType::DISCRETE_WINDING);
    curvatureTypeDropdown->addItem("Gradient of Arc Length", conis::core::CurvatureType::GRADIENT_ARC_LENGTH);
    curvatureTypeDropdown->addItem("Area Inflation", conis::core::CurvatureType::AREA_INFLATION);
    curvatureTypeDropdown->setToolTip(
            "<html><head/><body><p>Select the type of curvature calculation.</p></body></html>");

    connect(curvatureTypeDropdown, &QComboBox::currentIndexChanged, [this, curvatureTypeDropdown](int index) {
        viewSettings_.curvatureType = static_cast<conis::core::CurvatureType>(
                curvatureTypeDropdown->itemData(index).toInt());
        sceneView_->updateBuffers();
    });
    vertLayout->addWidget(new QLabel("Curvature Type"));
    vertLayout->addWidget(curvatureTypeDropdown);

    auto *curvatureScaleSlider = new DoubleSlider("Curvature Scale", viewSettings_.curvatureScale, 0.01, 10);
    curvatureScaleSlider->setToolTip(
            "<html><head/><body><p>Changes how long the curvature combs are.</p></body></html>");
    connect(curvatureScaleSlider, &DoubleSlider::valueUpdated, [this](double value) {
        viewSettings_.curvatureScale = value;
        sceneView_->updateBuffers();
    });
    vertLayout->addWidget(curvatureScaleSlider);
    double widthMultiplier = 100;
    auto *conicWidthSlider = new DoubleSlider("Conic Width", viewSettings_.conicWidth * widthMultiplier, 0.1, 10.0);
    conicWidthSlider->setToolTip(
            "<html><head/><body><p>Changes how thick the select conic is drawn.</p></body></html>");
    connect(conicWidthSlider, &DoubleSlider::valueUpdated, [this, widthMultiplier](double value) {
        viewSettings_.conicWidth = value / widthMultiplier;
        sceneView_->updateBuffers();
    });
    vertLayout->addWidget(conicWidthSlider);
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
    lightModeToggle->setIcon(QIcon(ImgResourceReader::getPixMap(":/icons/theme.png", {42, 42}, QColor(128, 128, 128))));
    lightModeToggle->setCheckable(true);
    lightModeToggle->setChecked(true); // default is light mode
    connect(lightModeToggle, &QAction::toggled, this, [this](bool toggled) {
        if (toggled) {
            applyStylePreset(viewSettings_, getLightModePalette());
        } else {
            applyStylePreset(viewSettings_, getDarkModePalette());
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
        QString filePath = QFileDialog::getOpenFileName(nullptr, "Load Curve", "../curves/", tr("Txt Files (*.txt)"));
        if (filePath == "") {
            return;
        }
        conis::core::CurveLoader loader;
        conis::core::Curve curve = loader.loadCurveFromFile(filePath.toStdString());
        auto &conisCurve = sceneView_->getConisCurve();
        viewSettings_.selectedVertex = -1;
        conisCurve.setControlCurve(curve);
        subdivStepsSpinBox->setVal(0);
        closedCurveAction->setChecked(curve.isClosed());
        sceneView_->viewToFit();
    });
    fileMenu->addAction(openAction);

    auto *saveAction = new QAction(QStringLiteral("Save"), fileMenu);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr,
                                                        "Save Image",
                                                        "../images/",
                                                        tr("Img Files (*.png *.jpg *.jpeg *.tiff *.tif *pgm *ppm)"));
        if (filePath != "") {
            QPixmap pixmap(sceneView_->size());
            sceneView_->render(&pixmap);
            bool success = pixmap.toImage().save(filePath);
            if (success) {
                QMessageBox::information(this, "Image Saved", filePath);
                return;
            }
            QMessageBox::warning(this, "Failed to save image", "Ensure you provided a file extension:\n:" + filePath);
            return;
        }
        QMessageBox::warning(this, "Failed to save image", "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveAction);

    // outputs list of points (x y coords) in a text file (for Lucia/Matlab)
    auto *saveCurveAction = new QAction(QStringLiteral("SaveCurve"), fileMenu);
    saveCurveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(saveCurveAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Curve", "../curves/", tr("TXT File (*.txt)"));
        if (filePath != "") {
            QByteArray bytes = filePath.toStdString().c_str();
            char *file_name;
            file_name = bytes.data();
            core::CurveSaver saver;
            auto &conisCurve = sceneView_->getConisCurve();
            bool success = saver.saveCurve(file_name, conisCurve.getSubdivCurve());

            if (success) {
                QMessageBox::information(this, "Curve Saved", filePath);
                return;
            }
            QMessageBox::warning(this, "Failed to save curve", "Ensure you provided a file extension:\n:" + filePath);
            return;
        }
        QMessageBox::warning(this, "Failed to save curve", "Ensure you provided a valid path:\n: " + filePath);
    });
    fileMenu->addAction(saveCurveAction);

    // outputs list of points and normals
    auto *saveCurveNAction = new QAction(QStringLiteral("SaveCurveN"), fileMenu);
    //    saveCurveNAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    connect(saveCurveNAction, &QAction::triggered, [this]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Curve", "../curves/", tr("Obj File (*.obj)"));
        if (filePath != "") {
            QByteArray bytes = filePath.toStdString().c_str();
            char *file_name;
            file_name = bytes.data();

            core::CurveSaver saver;
            auto &conisCurve = sceneView_->getConisCurve();
            bool success = saver.saveCurveWithNormals(file_name, conisCurve.getSubdivCurve());

            if (success) {
                QMessageBox::information(this, "Curve Saved", filePath);
                return;
            }
            QMessageBox::warning(this, "Failed to save curve", "Ensure you provided a file extension:\n:" + filePath);
            return;
        }
        QMessageBox::warning(this, "Failed to save curve", "Ensure you provided a valid path:\n: " + filePath);
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
        auto *newAction = new QAction(QString::fromStdString(name), presetMenu);
        newAction->setShortcut(QKeySequence::fromString(QString("Ctrl+%1").arg(i)));
        connect(newAction, &QAction::triggered, [this, name]() {
            auto &conisCurve = sceneView_->getConisCurve();
            presetName = name;
            viewSettings_.selectedVertex = -1;
            conisCurve.setControlCurve(presetFactory_.getPreset(presetName));
            presetLabel->setText(QString("<b>Preset:</b> %1").arg(QString::fromStdString(name)));
            subdivStepsSpinBox->setVal(0);
            closedCurveAction->setChecked(conisCurve.getControlCurve().isClosed());
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
    controlPointsAction->setChecked(viewSettings_.showControlPoints);
    controlPointsAction->setShortcut(QKeySequence(Qt::Key_P));
    connect(controlPointsAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.showControlPoints = toggled;
        sceneView_->updateBuffers();
    });
    renderMenu->addAction(controlPointsAction);

    auto *controlCurveAction = new QAction(QStringLiteral("Control Curve"), renderMenu);
    controlCurveAction->setCheckable(true);
    controlCurveAction->setChecked(viewSettings_.showControlCurve);
    controlCurveAction->setShortcut(QKeySequence(Qt::Key_O));
    connect(controlCurveAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.showControlCurve = toggled;
    });
    renderMenu->addAction(controlCurveAction);

    renderMenu->addSeparator();

    closedCurveAction = new QAction(QStringLiteral("Closed Curve"), renderMenu);
    closedCurveAction->setCheckable(true);
    closedCurveAction->setChecked(true);
    closedCurveAction->setShortcut(QKeySequence(Qt::Key_C));
    connect(closedCurveAction, &QAction::triggered, [this](bool toggled) {
        auto &conisCurve = sceneView_->getConisCurve();
        conisCurve.setControlCurveClosed(toggled);
    });
    renderMenu->addAction(closedCurveAction);

    renderMenu->addSeparator();

    auto *visualizeCurvatureAction = new QAction(QStringLiteral("Visualize Curvature"), renderMenu);
    visualizeCurvatureAction->setCheckable(true);
    visualizeCurvatureAction->setChecked(viewSettings_.visualizeCurvature);
    visualizeCurvatureAction->setShortcut(QKeySequence(Qt::Key_B));
    connect(visualizeCurvatureAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.visualizeCurvature = toggled;
        sceneView_->updateBuffers();
    });
    renderMenu->addAction(visualizeCurvatureAction);

    auto *visualizeNormalsAction = new QAction(QStringLiteral("Visualize Normals"), renderMenu);
    visualizeNormalsAction->setCheckable(true);
    visualizeNormalsAction->setChecked(viewSettings_.visualizeNormals);
    visualizeNormalsAction->setShortcut(QKeySequence(Qt::Key_N));
    connect(visualizeNormalsAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.visualizeNormals = toggled;
        sceneView_->updateBuffers();
    });
    renderMenu->addAction(visualizeNormalsAction);

    auto *normalHandlesAction = new QAction(QStringLiteral("Normal Handles"), renderMenu);
    normalHandlesAction->setCheckable(true);
    normalHandlesAction->setChecked(viewSettings_.normalHandles);
    normalHandlesAction->setShortcut(QKeySequence(Qt::Key_M));
    connect(normalHandlesAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.normalHandles = toggled;
        sceneView_->updateBuffers();
    });
    renderMenu->addAction(normalHandlesAction);

    auto *constrainNormalsAction = new QAction(QStringLiteral("Constrain Normal Movement"), renderMenu);
    constrainNormalsAction->setCheckable(true);
    constrainNormalsAction->setChecked(viewSettings_.constrainNormalMovement);
    connect(constrainNormalsAction, &QAction::triggered, [this](bool toggled) {
        viewSettings_.constrainNormalMovement = toggled;
    });
    renderMenu->addAction(constrainNormalsAction);
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

} // namespace conis::gui
