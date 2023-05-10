#include "src/core/conics/conicfitter.hpp"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVector2D>

#include <QDockWidget>

#include "src/ui/mainwindow.hpp"
#include "src/core/settings.hpp"

//#define COEF_TEST

void test() {
    int numPoints = 4;
    QVector<float> xCoords = {-4.8863, -4.3902, -0.4084, 0.0922};
    QVector<float> yCoords = {0.4696, -2.3533, 6.2764, 1.6286};
    QVector<float> xNorms = {-0.9592, -0.2420, -0.3349, 0.8824};
    QVector<float> yNorms = {0.2827, -0.9703, 0.9423, -0.4704};

    QVector<QVector2D> coords;
    QVector<QVector2D> normals;
    int i = 1;

    coords.append({xCoords[i], yCoords[i]});
    normals.append({xNorms[i], yNorms[i]});
    i = 2;

    coords.append({xCoords[i], yCoords[i]});
    normals.append({xNorms[i], yNorms[i]});
    i = 0;

    coords.append({xCoords[i], yCoords[i]});
    normals.append({xNorms[i], yNorms[i]});
    i = 3;

    coords.append({xCoords[i], yCoords[i]});
    normals.append({xNorms[i], yNorms[i]});
    //  for (int i = 1; i < 2; i++) {
    //    coords.append({xCoords[i], yCoords[i]});
    //    normals.append({xNorms[i], yNorms[i]});
    //  }
    //  for (int i = 0; i < numPoints; i++) {
    //    coords.append({xCoords[i], yCoords[i]});
    //    normals.append({xNorms[i], yNorms[i]});
    //  }
    Settings settings;
    settings.pointWeight = 100000;
    settings.normalWeight = 100000;
    ConicFitter fitter = ConicFitter();
    QVector<double> foundCoefs = fitter.fitConic(coords, normals, settings);
    qDebug() << foundCoefs;
}

/**
 * @brief main Starts up the QT application and UI.
 * @param argc Argument count.
 * @param argv Arguments.
 * @return Exit code.
 */
int main(int argc, char *argv[]) {
#ifdef COEF_TEST
    test();
    return EXIT_SUCCESS;
#else
    QApplication app(argc, argv);

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
#endif
}
