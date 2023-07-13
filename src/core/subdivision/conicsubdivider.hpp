#pragma once


#include "core/settings.hpp"
#include "util/vector.hpp"

class SubdivisionCurve;

class ConicSubdivider {
public:
    explicit ConicSubdivider(const Settings &settings);

    void subdivide(SubdivisionCurve *curve, int level);

    void knotCurve(SubdivisionCurve *curve, std::vector<Vector2DD> &coords, std::vector<Vector2DD> &norms,
                   std::vector<bool> &customNorms);

    [[nodiscard]] std::vector<double> getStabilityVals() const;

private:
    const Settings &settings_;
    std::vector<double> stability_;
    SubdivisionCurve *curve_;

    void subdivide(const std::vector<Vector2DD> &points,
                   const std::vector<Vector2DD> &normals,
                   const std::vector<double> &stabilities,
                   int level);


    void extractPatch(const std::vector<Vector2DD> &points,
                      const std::vector<Vector2DD> &normals,
                      int i,
                      std::vector<PatchPoint> &patchPoints) const;


    [[nodiscard]] bool areInSameHalfPlane(const Vector2DD &v0,
                                          const Vector2DD &v1,
                                          const Vector2DD &v2,
                                          const Vector2DD &v3) const;
};
