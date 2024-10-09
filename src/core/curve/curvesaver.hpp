#pragma once

#include <QString>

#include "core/curve/curve.hpp"

namespace conics::core {

class CurveSaver {

public:
    CurveSaver();

    bool saveCurve(const char *fileName, const Curve &curve) const;
    bool saveCurveWithNormals(const char *fileName, const Curve &curve) const ;
};

} // namespace conics::core