#include <QApplication>
#include <QSurfaceFormat>

#include <QDockWidget>

#include "gui/mainwindow.hpp"
#include "core/scene.hpp"
#include "core/settings/settings.hpp"

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

    conics::core::Settings settings;
    conics::core::Scene scene(settings);

    conics::ui::MainWindow w(settings, scene);
    w.showMaximized();
    return QApplication::exec();
}
