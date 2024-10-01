#include "curveloader.hpp"

#include <QDebug>
#include <QFile>
#include <QMap>
#include <QTextStream>

namespace conics::core {

CurveLoader::CurveLoader() {}

static void insertLineSegment(QMap<int, int> &lineSegments, int startIdx, int endIdx) {
    if (!lineSegments.contains(startIdx)) {
        lineSegments.insert(startIdx, endIdx);
        return;
    }
    int newEndIdx = lineSegments[startIdx];
    insertLineSegment(lineSegments, newEndIdx, startIdx);
    lineSegments.insert(startIdx, endIdx);
}

Curve CurveLoader::loadCurveFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return Curve();
    }

    std::vector<Vector2DD> coords, normals;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if (parts.isEmpty())
            continue;

        QString type = parts[0];

        if (type == "v") {
            if (parts.size() >= 3) {
                Vector2DD vertex;
                vertex.x() = parts[1].toDouble();
                vertex.y() = parts[2].toDouble();
                coords.emplace_back(vertex);
            }
        } else if (type == "vn") {
            if (parts.size() >= 3) {
                Vector2DD normal;
                normal.x() = parts[1].toDouble();
                normal.y() = parts[2].toDouble();
                normal /= sqrtl(normal.x() * normal.x() + normal.y() * normal.y());
                normals.emplace_back(normal);
            }
        }
    }

    bool closed = true;

    file.close();
    if (normals.empty()) {
        return Curve(coords, closed);
    }
    return Curve(coords, normals, closed);
}

} // namespace conics::core