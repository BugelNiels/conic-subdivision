#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

#include "external/qt-value-slider/include/doubleslider.hpp"
#include "external/qt-value-slider/include/intslider.hpp"

#include "conis/core/curve/curvepresetfactory.hpp"
#include "conis/core/coniscurve.hpp"
#include "conis/gui/viewsettings.hpp"

namespace conis::gui {

class SceneView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(conis::core::ConisCurve &scene,
                        conis::core::SubdivisionSettings &subdivSettings,
                        conis::core::NormalRefinementSettings &normRefSettings,
                        ViewSettings &viewSettings,
                        QWidget *parent = nullptr);

    ~MainWindow() override = default;

private:
    const int maxWeight = 10E8;
    SceneView *sceneView_;
    ViewSettings &viewSettings_;
    conis::core::SubdivisionSettings &subdivSettings_;
    conis::core::NormalRefinementSettings &normRefSettings_;
    conis::core::CurvePresetFactory presetFactory_;

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

    void resetView(bool recalculate = true);
};

} // namespace conis::gui