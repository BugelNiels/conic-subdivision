#pragma once

#include "core/curve/curve.hpp"

namespace conics::core {

class Subdivider {
public:
    /**
     * Subdivides the curve to the given subdivision level using the Conic subdivision scheme.
     * @param curve The curve to subdivide.
     * @param level The level to subdivide to.
     */
    virtual void subdivide(Curve &curve, int level) = 0;
};

} // namespace conics::core