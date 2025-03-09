#pragma once

#include "conis/core/curve/curve.hpp"

namespace conis::core {

class CurveSaver {

public:
    CurveSaver();

    bool saveCurve(const std::string &fileName, const Curve &curve) const;
    bool saveCurveWithNormals(const std::string &fileName, const Curve &curve) const;
};

} // namespace conis::core
