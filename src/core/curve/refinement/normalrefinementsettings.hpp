#pragma once

#include "core/vector.hpp"

namespace conics::core {

using NormalRefinementSettings = struct NormalRefinementSettings {
    int maxRefinementIterations = 1;
    real_t angleLimit = 1.0e-8;
    int testSubdivLevel = 6;
};

} // namespace conics::core