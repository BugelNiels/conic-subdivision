#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

#include "external/qt-value-slider/include/doubleslider.hpp"
#include "external/qt-value-slider/include/intslider.hpp"

#include "core/curve/curvepresetfactory.hpp"
#include "core/scene.hpp"
#include "core/settings/settings.hpp"

namespace conics::ui {

class MainView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(conics::core::Settings& settings, conics::core::Scene &scene, QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    const int maxWeight = 10E8;

    QString presetName;
    QAction *closedCurveAction;
    QDockWidget *dock_;
    QLabel *presetLabel;
    ValueSliders::IntSlider *subdivStepsSpinBox;

    conics::core::CurvePresetFactory presetFactory_;
    conics::core::Settings& settings_;
    MainView *mainView_;

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenu *getFileMenu();

    QMenu *getWindowMenu();

    void resetView(bool recalculate = true);
};

} // namespace conics::ui