#pragma once

#include "core/subdivisioncurve.hpp"
#include <QString>

class ObjCurveReader {

public:
    explicit ObjCurveReader(const Settings &settings);

    SubdivisionCurve loadCurveFromObj(const QString &filePath);

private:
    const Settings &settings_;
};
