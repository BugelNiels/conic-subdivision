#include <QApplication>
#include <QSurfaceFormat>

#include <QDockWidget>

#include "gui/mainwindow.hpp"
#include "core/scene.hpp"
#include "core/curve/refinement/normalrefinementsettings.hpp"
#include "core/curve/subdivision/subdivisionsettings.hpp"
#include "gui/viewsettings.hpp"

/**
 * @brief main Starts up the QT application and UI.
 * @param argc Argument count.
 * @param argv Arguments.
 * @return Exit code.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qDebug() << "Using " << sizeof(conics::core::real_t) << "byte calculations";

    QSurfaceFormat glFormat;
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    glFormat.setVersion(4, 1);
    glFormat.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(glFormat);

    QFont font("Lato");
    font.setPixelSize(14);
    QApplication::setFont(font);

    conics::core::SubdivisionSettings subdivSettings;
    conics::core::NormalRefinementSettings normRefSettings;
    conics::gui::ViewSettings viewSettings;
    conics::core::Scene scene(subdivSettings, normRefSettings);

    conics::gui::MainWindow w(scene, subdivSettings, normRefSettings, viewSettings);
    w.showMaximized();
    return QApplication::exec();
}
