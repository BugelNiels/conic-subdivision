#include <QApplication>
#include <QSurfaceFormat>

#include <QDockWidget>

#include "src/gui/mainwindow.hpp"

/**
 * @brief main Starts up the QT application and UI.
 * @param argc Argument count.
 * @param argv Arguments.
 * @return Exit code.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qDebug() << "Using " << sizeof(real_t) << "bit calculations";

    QSurfaceFormat glFormat;
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    glFormat.setVersion(4, 1);
    glFormat.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(glFormat);

    QFont font("Lato");
    font.setPixelSize(14);
    QApplication::setFont(font);

    MainWindow w;
    w.showMaximized();
    return QApplication::exec();
}
