#pragma once

#include "src/core/settings.hpp"
#include "util/vector.hpp"
#include "src/core/subdivisioncurve.hpp"

class NormalRefiner {
public:
    explicit NormalRefiner(int maxIter);

    std::vector<Vector2DD> refine(const SubdivisionCurve& curve) const;

private:
    int maxIter = 0;

};
