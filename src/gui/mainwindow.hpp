#pragma once

#include "core/settings.hpp"
#include "external/qt-value-slider/include/doubleslider.hpp"
#include "external/qt-value-slider/include/intslider.hpp"
#include "src/core/conics/conicpresets.hpp"
#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

class MainView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    const int maxWeight = 10E8;

    conics::ConicPresets *presets_;
    Settings settings_;
    MainView *mainView_;

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getFileMenu();

    QLabel *presetLabel;

    void resetView(bool recalculate = true);

    ValueSliders::IntSlider *subdivStepsSpinBox;
    QAction *closedCurveAction;
    QString presetName;

    QMenu *getWindowMenu();

    QDockWidget *dock_;
};
