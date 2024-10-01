#include "imgresourcereader.hpp"

#include <QPainter>

namespace conics::util {

QMap<QString, QPixmap> ImgResourceReader::loadedIcons_;

QPixmap ImgResourceReader::getPixMap(const QString &path, const QSize &size, const QColor &color) {
    QString key = QString("%1?color=%2&size=%3x%4")
                          .arg(path)
                          .arg(color.name())
                          .arg(size.width())
                          .arg(size.height());
    if (loadedIcons_.contains(key)) {
        return ImgResourceReader::loadedIcons_[key];
    }
    QImage myImage(path);

    // TODO: check if this can be done faster.
    QSize size2 = myImage.size();

    for (int x = 0; x < size2.width(); x++) {
        for (int y = 0; y < size2.height(); y++) {
            int alpha = myImage.pixelColor(x, y).alpha();
            myImage.setPixelColor(x, y, QColor(color.red(), color.green(), color.blue(), alpha));
        }
    }

    auto newIcon = QPixmap::fromImage(myImage).scaled(size, Qt::AspectRatioMode::KeepAspectRatio);
    loadedIcons_[key] = newIcon;
    return newIcon;
}

} // namespace conics::util