#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

#include "external/qt-value-slider/include/doubleslider.hpp"
#include "external/qt-value-slider/include/intslider.hpp"

#include "core/conics/conicpresetfactory.hpp"
#include "core/settings/settings.hpp"

class MainView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    const int maxWeight = 10E8;

    QString presetName;
    QAction *closedCurveAction;
    QDockWidget *dock_;
    QLabel *presetLabel;
    ValueSliders::IntSlider *subdivStepsSpinBox;

    conics::ConicPresetFactory presetFactory_;
    Settings settings_;
    MainView *mainView_;

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenu *getFileMenu();

    QMenu *getWindowMenu();

    void resetView(bool recalculate = true);

};
