#pragma once


#include <QString>
#include "core/subdivisioncurve.hpp"

class ObjCurveReader {

public:
    ObjCurveReader(const Settings& settings);

    SubdivisionCurve loadCurveFromObj(const QString &filePath);

private:
    const Settings& settings_;
};

