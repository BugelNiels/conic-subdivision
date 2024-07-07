#pragma once

#include "src/core/settings.hpp"
#include "util/vector.hpp"
#include "src/core/subdivisioncurve.hpp"

class NormalRefiner {
public:
    explicit NormalRefiner(int maxIter);

    void refine(SubdivisionCurve& curve) const;
    void refineSelected(SubdivisionCurve& curve, int idx) const;

private:
    int maxIter = 1;

};
