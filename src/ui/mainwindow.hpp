#pragma once

#include <QMainWindow>
#include <QLabel>
#include "src/core/conics/conicpresets.hpp"

class MainView;

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:

    conics::ConicPresets *presets_;
    Settings *settings_;
    MainView *mainView_;

    QMenu *getPresetMenu();

    QMenu *getRenderMenu();

    QMenuBar *initMenuBar();

    QDockWidget *initSideMenu();

    QMenu *getFileMenu();

    QLabel *presetLabel;

    void resetView();
};
