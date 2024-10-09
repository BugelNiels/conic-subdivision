#pragma once

#include "core/curve/curve.hpp"

namespace conis::core {

class CurveLoader {

public:
    CurveLoader();

    Curve loadCurveFromFile(const std::string &filePath);
};

} // namespace conis::core