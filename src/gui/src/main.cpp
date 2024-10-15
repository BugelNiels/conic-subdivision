#include <QApplication>
#include <QSurfaceFormat>

#include "mainwindow.hpp"
#include "conis/core/coniscurve.hpp"
#include "viewsettings.hpp"

/**
 * @brief main Starts up the QT application and UI.
 * @param argc Argument count.
 * @param argv Arguments.
 * @return Exit code.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    qDebug() << "Using " << sizeof(conis::core::real_t) << "byte calculations";

    QSurfaceFormat glFormat;
    glFormat.setProfile(QSurfaceFormat::CoreProfile);
    glFormat.setVersion(4, 1);
    glFormat.setOption(QSurfaceFormat::DebugContext);
    QSurfaceFormat::setDefaultFormat(glFormat);

    QFont font("Lato");
    font.setPixelSize(14);
    QApplication::setFont(font);

    conis::core::SubdivisionSettings subdivSettings;
    conis::core::NormalRefinementSettings normRefSettings;
    conis::gui::ViewSettings viewSettings;
    conis::core::ConisCurve conisCurve(subdivSettings, normRefSettings);

    conis::gui::MainWindow w(conisCurve, subdivSettings, normRefSettings, viewSettings);
    w.showMaximized();
    return QApplication::exec();
}
