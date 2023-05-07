#include "mainwindow.hpp"

#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QToolTip>
#include <QOpenGLFramebufferObject>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLPaintDevice>

#include "src/core/conicpresets.hpp"
#include "ui/stylepresets.hpp"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent) {

    settings = new Settings();
    mainView = new MainView(settings, this);


    auto *dock = initSideMenu();
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    setMenuBar(initMenuBar());
    setCentralWidget(mainView);

    conics::ui::applyStylePreset(*settings, conics::ui::getLightModePalette());

    mainView->setSubCurve(std::make_shared<SubdivisionCurve>(presets.getPreset(presets.getPresetNames().at(0))));
}


MainWindow::~MainWindow() {
}


QDockWidget *MainWindow::initSideMenu() {
    QWidget *sideMenu = new QWidget(this);
    auto *vertLayout = new QVBoxLayout(sideMenu);
    vertLayout->addWidget(new QLabel("Subdivision Steps"));
    auto *subdivStepsSpinBox = new QSpinBox();
    subdivStepsSpinBox->setMinimum(0);
    subdivStepsSpinBox->setMaximum(20);
    connect(subdivStepsSpinBox, &QSpinBox::valueChanged, [this](int numSteps) {
        mainView->subdivideCurve(numSteps);
        mainView->updateBuffers();
    });
    vertLayout->addWidget(subdivStepsSpinBox);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Vertex Weights"));
    auto *edgeVertWeightSpinBox = new QDoubleSpinBox();
    edgeVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the points at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>");
    edgeVertWeightSpinBox->setMinimum(0);
    edgeVertWeightSpinBox->setMaximum(100000);
    edgeVertWeightSpinBox->setValue(settings->pointWeight);
    connect(edgeVertWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->pointWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(edgeVertWeightSpinBox);

    auto *midVertWeightSpinBox = new QDoubleSpinBox();
    midVertWeightSpinBox->setMinimum(0);
    midVertWeightSpinBox->setMaximum(100000);
    midVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the points at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    midVertWeightSpinBox->setValue(settings->middlePointWeight);
    connect(midVertWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->middlePointWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(midVertWeightSpinBox);

    auto *outerVertWeightSpinBox = new QDoubleSpinBox();
    outerVertWeightSpinBox->setMinimum(0);
    outerVertWeightSpinBox->setMaximum(100000);
    outerVertWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p><span style=&quot; font-weight:600;&quot;>a</span>-b-<span style=&quot; font-weight:600;&quot;>c-</span>d-e-<span style=&quot; font-weight:600;&quot;>f</span></p><p>this value change the weights of the points <span style=&quot; font-weight:600;&quot;>a</span> and <span style=&quot; font-weight:600;&quot;>f </span>(the end points). </p></body></html>"
    );
    outerVertWeightSpinBox->setValue(settings->outerPointWeight);
    connect(outerVertWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->outerPointWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(outerVertWeightSpinBox);
    vertLayout->addStretch();

    vertLayout->addWidget(new QLabel("Normal weights"));
    auto *edgeNormWeightSpinBox = new QDoubleSpinBox();
    edgeNormWeightSpinBox->setMinimum(0);
    edgeNormWeightSpinBox->setMaximum(100000);
    edgeNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-b-<span style=&quot; font-weight:600;&quot;>c-d</span>-e-f</p><p>this value changes the weights of the normals at <span style=&quot; font-weight:600;&quot;>c</span> and <span style=&quot; font-weight:600;&quot;>d </span>(the edge points).</p></body></html>"
    );
    edgeNormWeightSpinBox->setValue(settings->normalWeight);
    connect(edgeNormWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->normalWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(edgeNormWeightSpinBox);

    auto *midNormWeightSpinBox = new QDoubleSpinBox();
    midNormWeightSpinBox->setMinimum(0);
    midNormWeightSpinBox->setMaximum(100000);
    midNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p>a-<span style=&quot; font-weight:600;&quot;>b</span>-c<span style=&quot; font-weight:600;&quot;>-</span>d-<span style=&quot; font-weight:600;&quot;>e</span>-f</p><p>this value change the weights of the normals at <span style=&quot; font-weight:600;&quot;>b</span> and <span style=&quot; font-weight:600;&quot;>e</span>.</p></body></html>"
    );
    midNormWeightSpinBox->setValue(settings->middleNormalWeight);
    connect(midNormWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->middleNormalWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(midNormWeightSpinBox);

    auto *outerNormWeightSpinBox = new QDoubleSpinBox();
    outerNormWeightSpinBox->setMinimum(0);
    outerNormWeightSpinBox->setMaximum(100000);
    outerNormWeightSpinBox->setToolTip(
            "<html><head/><body><p>In the line segment </p><p><span style=&quot; font-weight:600;&quot;>a</span>-b-<span style=&quot; font-weight:600;&quot;>c-</span>d-e-<span style=&quot; font-weight:600;&quot;>f</span></p><p>this value change the weights of the normals at <span style=&quot; font-weight:600;&quot;>a</span> and <span style=&quot; font-weight:600;&quot;>f </span>(the end points). </p></body></html>"
    );
    outerNormWeightSpinBox->setValue(settings->outerNormalWeight);
    connect(outerNormWeightSpinBox, &QDoubleSpinBox::valueChanged, [this](double newVal) {
        settings->outerNormalWeight = newVal;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(outerNormWeightSpinBox);

    vertLayout->addStretch();
    auto *normSolveCheckBox = new QCheckBox("Normalized Solve");
    normSolveCheckBox->setToolTip(
            "<html><head/><body><p>Enabling this will try to fit a conic using the unit normal constraint. If this option is disabled, a separated scaling value for each normal is calculated.</p></body></html>"
    );
    normSolveCheckBox->setChecked(settings->normalizedSolve);
    connect(normSolveCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings->normalizedSolve = toggled;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(normSolveCheckBox);

    auto *circleNormsCheckBox = new QCheckBox("Circle Normals");
    circleNormsCheckBox->setToolTip(
            "<html><head/><body><p>Estimate the normals using oscilating circles.</p></body></html>"
    );
    circleNormsCheckBox->setChecked(settings->circleNormals);
    connect(circleNormsCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings->circleNormals = toggled;
        mainView->recalculateCurve();
    });

    auto *recalcNormsCheckBox = new QCheckBox("Recalculate Normals");
    recalcNormsCheckBox->setToolTip(
            "<html><head/><body><p>If this option is enabled, the normals will be re-evaluated at every step. If this option is disabled, the vertex points will keep their normals. Any new edge points will obtain the normal of the conic they were sampled from.</p></body></html>"
    );
    recalcNormsCheckBox->setChecked(settings->recalculateNormals);
    connect(recalcNormsCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings->recalculateNormals = toggled;
        mainView->recalculateCurve();
    });
    vertLayout->addWidget(recalcNormsCheckBox);

    auto *edgeSampleCheckBox = new QCheckBox("Edge Normal");
    edgeSampleCheckBox->setToolTip(
            "<html><head/><body><p>If enabled, uses a ray perpendical to the edge to intersect with the conic and sample the new point from. If disabled, uses the average of the edge point normals.</p><p><br/></p><p>In both cases, the ray originates from the middle of the edge.</p></body></html>"
    );
    edgeSampleCheckBox->setChecked(settings->edgeTangentSample);
    connect(edgeSampleCheckBox, &QCheckBox::toggled, [this](bool toggled) {
        settings->edgeTangentSample = toggled;
        mainView->recalculateCurve();
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
    return menuBar;
}

QMenu *MainWindow::getFileMenu() {
    auto *fileMenu = new QMenu("File");

    auto *newAction = new QAction(QStringLiteral("New"), this);
    newAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(newAction, &QAction::triggered, [this]() {
        mainView->setSubCurve(std::make_shared<SubdivisionCurve>(presets.getPreset(presets.getPresetNames().at(0))));
        mainView->recalculateCurve(); // TODO: resit settings
    });
    fileMenu->addAction(newAction);


    auto *saveAction = new QAction(QStringLiteral("Save"), this);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    connect(saveAction, &QAction::triggered, [this]() {
        // TODO
//        QPixmap pixmap(mainView->size());
//        mainView->render(&pixmap);
//
//        QOpenGLFramebufferObject fbo(mainView->size());
//        fbo.bind();
//        QOpenGLPaintDevice d(mainView->size());
//        QPainter painter(&d);
//        painter.beginNativePainting();
//        mainView->render(&painter);
////        mainView->paintGL();
//
//        painter.endNativePainting();
//        painter.end();
//        fbo.release();
//
//        QImage image = fbo.toImage();
////        return;
//        QString filePath = QFileDialog::getSaveFileName(
//                nullptr, "Save Image", "../images/",
//                tr("Img Files (*.png *.jpg *.jpeg *.tiff *.tif *pgm *ppm)"));
//
//        if (filePath != "") {
//            bool success = image.save(filePath);
//            if (success) {
//                QMessageBox::information(this, "Image Saved", filePath);
//                return;
//            }
//        }
//        QMessageBox::warning(this, "Failed to save image", filePath);
    });
    fileMenu->addAction(saveAction);

    auto *quitAction = new QAction(QStringLiteral("Quit"), this);
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
    for (auto &name: presets.getPresetNames()) {
        auto *newAction = new QAction(name, this);
        newAction->setShortcut(QKeySequence::fromString(QString("Ctrl+%1").arg(i)));
        connect(newAction, &QAction::triggered, [this, name]() {
            mainView->setSubCurve(std::make_shared<SubdivisionCurve>(presets.getPreset(name)));
            mainView->recalculateCurve();
        });
        presetMenu->addAction(newAction);
        i++;
    }

    return presetMenu;
}

QMenu *MainWindow::getRenderMenu() {
    auto *renderMenu = new QMenu("View");

    auto *controlPointsAction = new QAction(QStringLiteral("Control Points"), this);
    controlPointsAction->setCheckable(true);
    controlPointsAction->setChecked(settings->showControlPoints);
    controlPointsAction->setShortcut(QKeySequence(Qt::Key_P));
    connect(controlPointsAction, &QAction::triggered, [this](bool toggled) {
        settings->showControlPoints = toggled;
        mainView->updateBuffers();
    });
    renderMenu->addAction(controlPointsAction);

    auto *controlCurveAction = new QAction(QStringLiteral("Control Curve"), this);
    controlCurveAction->setCheckable(true);
    controlCurveAction->setChecked(settings->showControlCurve);
    controlCurveAction->setShortcut(QKeySequence(Qt::Key_O));
    connect(controlCurveAction, &QAction::triggered, [this](bool toggled) {
        settings->showControlCurve = toggled;
        mainView->updateBuffers();
    });
    renderMenu->addAction(controlCurveAction);

    renderMenu->addSeparator();

    auto *closedCurveAction = new QAction(QStringLiteral("Closed Curve"), this);
    closedCurveAction->setCheckable(true);
    closedCurveAction->setChecked(settings->closed);
    closedCurveAction->setShortcut(QKeySequence(Qt::Key_C));
    connect(closedCurveAction, &QAction::triggered, [this](bool toggled) {
        settings->closed = toggled;
        mainView->updateBuffers();
    });
    renderMenu->addAction(closedCurveAction);

    renderMenu->addSeparator();

    auto *visualizeNormalsAction = new QAction(QStringLiteral("Visualize Normals"), this);
    visualizeNormalsAction->setCheckable(true);
    visualizeNormalsAction->setChecked(settings->visualizeNormals);
    visualizeNormalsAction->setShortcut(QKeySequence(Qt::Key_N));
    connect(visualizeNormalsAction, &QAction::triggered, [this](bool toggled) {
        settings->visualizeNormals = toggled;
        mainView->updateBuffers();
    });
    renderMenu->addAction(visualizeNormalsAction);

    auto *flipNormals = new QAction(QStringLiteral("Flip Normals"), this);
    flipNormals->setShortcut(QKeySequence(Qt::ALT | Qt::Key_N));
    connect(flipNormals, &QAction::triggered, [this]() {
        settings->outwardNormals = !settings->outwardNormals;
        mainView->recalculateCurve();
        mainView->updateBuffers();
    });
    renderMenu->addAction(flipNormals);
    return renderMenu;
}
