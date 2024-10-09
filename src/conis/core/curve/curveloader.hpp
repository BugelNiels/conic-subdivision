#pragma once

#include "conis/core/curve/curve.hpp"

namespace conis::core {

class CurveLoader {

public:
    CurveLoader();

    Curve loadCurveFromFile(const std::string &filePath);
};

} // namespace conis::core