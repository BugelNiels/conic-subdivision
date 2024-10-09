#pragma once

#include "core/vector.hpp"

namespace conis::core {

using NormalRefinementSettings = struct NormalRefinementSettings {
    int maxRefinementIterations = 1;
    real_t angleLimit = 1.0e-8;
    int testSubdivLevel = 6;
};

} // namespace conis::core