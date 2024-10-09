#pragma once

#include <QString>

#include "core/curve/curve.hpp"

namespace conics::core {

class CurveLoader {

public:
    CurveLoader();

    Curve loadCurveFromFile(const QString &filePath);
};

} // namespace conics::core