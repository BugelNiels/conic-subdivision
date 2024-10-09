#pragma once

#include "core/vector.hpp"

namespace conics::core {

using SubdivisionSettings = struct SubdivisionSettings {
    // Calculation Weights
    real_t middlePointWeight = 100000.0;
    real_t middleNormalWeight = 100000.0;
    // The ratio between this and outerNormalWeight is Tau in the paper
    real_t outerPointWeight = 100.0;
    real_t outerNormalWeight = 1.0;
    int patchSize = 2;

    // Calculation settings
    bool circleNormals = false;
    bool areaWeightedNormals = true;
    bool convexitySplit = true;
    bool weightedInflPointLocation = false;
    bool gravitateSmallerAngles = true;
    real_t epsilon = 1e-40;
    bool dynamicPatchSize = true;
    
    // For testing purposes
    bool testToggle = false;
};

} // namespace conics::core