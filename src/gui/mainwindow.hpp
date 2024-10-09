#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSpinBox>

#include "external/qt-value-slider/include/doubleslider.hpp"
#include "external/qt-value-slider/include/intslider.hpp"

#include "core/curve/curvepresetfactory.hpp"
#include "core/curve/refinement/normalrefinementsettings.hpp"
#include "core/curve/subdivision/subdivisionsettings.hpp"
#include "core/scene.hpp"
#include "gui/viewsettings.hpp"

namespace conics::gui {

class SceneView;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(conics::core::Scene &scene,
                        conics::core::SubdivisionSettings &subdivSettings,
                        conics::core::NormalRefinementSettings &normRefSettings,
                        ViewSettings &viewSettings,
                        QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    const int maxWeight = 10E8;
    SceneView *sceneView_;
    ViewSettings &viewSettings_;
    conics::core::SubdivisionSettings &subdivSettings_;
    conics::core::NormalRefinementSettings &normRefSettings_;
    conics::core::CurvePresetFactory presetFactory_;

    QString presetName;
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

} // namespace conics::gui