#pragma once

#include <QLabel>
#include <QMainWindow>

#include "qt-value-slider/include/doubleslider.hpp"
#include "qt-value-slider/include/intslider.hpp"



#include "conis/core/curve/curvepresetfactory.hpp"
#include "conis/core/coniscurve.hpp"
#include "viewsettings.hpp"

namespace conis::gui {

class SceneView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(core::ConisCurve &scene,
                        core::SubdivisionSettings &subdivSettings,
                        core::NormalRefinementSettings &normRefSettings,
                        ViewSettings &viewSettings,
                        QWidget *parent = nullptr);

    ~MainWindow() override = default;

private:
    const int maxWeight = 10E8;
    SceneView *sceneView_;
    ViewSettings &viewSettings_;
    core::SubdivisionSettings &subdivSettings_;
    core::NormalRefinementSettings &normRefSettings_;
    core::CurvePresetFactory presetFactory_;

    std::string presetName;
    QAction *closedCurveAction;
    QDockWidget *dock_;
    QLabel *presetLabel;
    ValueSliders::IntSlider *subdivStepsSpinBox;

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenu *getFileMenu();

    QMenu *getWindowMenu();

    void resetView();
};

} // namespace conis::gui