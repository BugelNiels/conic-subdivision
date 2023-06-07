#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QSpinBox>
#include "src/core/conics/conicpresets.hpp"
#include "core/settings.hpp"

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

    QSpinBox *subdivStepsSpinBox;
    QAction *closedCurveAction;
    QString presetName;

    QMenu *getWindowMenu();

    QDockWidget *dock_;
};
