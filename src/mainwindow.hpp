#pragma once

#include <QMainWindow>
#include "mainview.hpp"
#include "core/conicpresets.hpp"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:

    conics::ConicPresets presets;
    Settings *settings;
    MainView *mainView;

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getFileMenu();
};
