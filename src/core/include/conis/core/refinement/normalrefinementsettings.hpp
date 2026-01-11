#pragma once

#include "conis/core/vector.hpp"

namespace conis::core {

using NormalRefinementSettings = struct NormalRefinementSettings {
    int maxRefinementIterations = 1;
    real_t angleLimit = 0.00001;
    int testSubdivLevel = 6;
};

} // namespace conis::core
