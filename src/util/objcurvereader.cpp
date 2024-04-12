#include "objcurvereader.hpp"
#include <QFile>

ObjCurveReader::ObjCurveReader(const Settings &settings) : settings_(settings) {}

static void insertLineSegment(QMap<int, int> &lineSegments, int startIdx, int endIdx) {
    if (!lineSegments.contains(startIdx)) {
        lineSegments.insert(startIdx, endIdx);
        return;
    }
    int newEndIdx = lineSegments[startIdx];
    insertLineSegment(lineSegments, newEndIdx, startIdx);
    lineSegments.insert(startIdx, endIdx);
}

// Niels' version for his .obj files from Blender (with unordered line-segments)
//SubdivisionCurve ObjCurveReader::loadCurveFromObj(const QString &filePath) {
//    QFile file(filePath);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "Failed to open file:" << file.errorString();
//        return SubdivisionCurve(settings_);
//    }

//    std::vector<Vector2DD> coords, normals;
//    QMap<int, int> lineSegments;
//    QTextStream in(&file);
//    while (!in.atEnd()) {
//        QString line = in.readLine().trimmed();
//        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

//        if (parts.isEmpty())
//            continue;

//        QString type = parts[0];

//        if (type == "v") {
//            if (parts.size() >= 3) {
//                Vector2DD vertex;
//                vertex.x() = parts[1].toFloat();
//                vertex.y() = parts[2].toFloat();
//                coords.emplace_back(vertex);
//            }
//        } else if (type == "vn") {
//            if (parts.size() >= 3) {
//                Vector2DD normal;
//                normal.x() = parts[1].toFloat();
//                normal.y() = parts[2].toFloat();
//                normals.emplace_back(normal);
//            }
//        } else if (type == "l") {
//            // Indices start at 1, so offset here immediately
//            int startIdx = parts[1].toInt() - 1;
//            int endIdx = parts[2].toInt() - 1;
//            insertLineSegment(lineSegments, startIdx, endIdx);
//        }
//    }
//    std::vector<Vector2DD> orderedVertices;
//    int startVertex = lineSegments.firstKey();
//    int currentVertex = startVertex;

//    bool closed = false;

//    while (!lineSegments.isEmpty()) {
//        orderedVertices.emplace_back(coords[currentVertex]);
//        int nextVertex = lineSegments.take(currentVertex);
//        if (nextVertex == startVertex) {
//            closed = true;
//            break;
//        }
//        if (nextVertex < 0) {
//            // If the next vertex is < 0, it indicates an invalid line segment
//            qDebug() << "Invalid line segment detected!";
//            break;
//        }
//        currentVertex = nextVertex;
//    }
//    file.close();
//    if (normals.empty()) {
//        return SubdivisionCurve(settings_, orderedVertices, closed);
//    }
//    return SubdivisionCurve(settings_, orderedVertices, normals, closed);
//}

SubdivisionCurve ObjCurveReader::loadCurveFromObj(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << file.errorString();
        return SubdivisionCurve(settings_);
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
        return SubdivisionCurve(settings_, coords, closed);
    }
    return SubdivisionCurve(settings_, coords, normals, closed);
}
