#pragma once

#include "core/curve.hpp"
#include <QString>

class ObjCurveReader {

public:
    ObjCurveReader();

    Curve loadCurveFromObj(const QString &filePath);
};
