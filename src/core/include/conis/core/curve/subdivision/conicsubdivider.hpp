#pragma once

#include "conis/core/curve/curve.hpp"
#include "conis/core/curve/subdivision/subdivisionsettings.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

class ConicSubdivider {
public:
    explicit ConicSubdivider(const SubdivisionSettings &settings);

    /**
     * Subdivides the curve to the given subdivision level using the Conic subdivision scheme. Subdivision happens in-place.
     * @param curve The curve to subdivide.
     * @param level The level to subdivide to.
     */
    void subdivide(Curve &curve, int level);

    /**
     * Returns a curve with the inflection points inserted such that the curve can be split into globally convex segments.
     * @param curve The curve that stores the information on the inflection point indices.
     * @return A curve with the inflection points inserted.
     */
    Curve getInflPointCurve(const Curve &curve);

    /**
     * Extracts a patch of a given neighbourhood from a curve. Maintains convexity provided that inflection points were inserted.
     * @param curve The curve to extract the patch from
     * @param i The index of the first vertex of the edge to find the patch for. That is, for the edge A-B, i denotes the index of A.
     * @param maxPatchSize The maximum number of points allowed in the patch on either side of the edge.
     * The total maximum size of the patch will be 2 * maxPatchSize.
     * For example, maxPatchSize = 1 means only the edge points are included, while maxPatchSize = 2 means a total maximum size of 4.
     * @return A collection of patch points.
     */
    std::vector<PatchPoint> extractPatch(const Curve &curve, int pIdx, int maxPatchSize) const;

private:
    const SubdivisionSettings &settings_;
    std::vector<int> inflPointIndices_;
    // These buffers persist between subdivisions to prevent re-allocation
    Curve bufferCurve_;

    void subdivideRecursive(Curve &controlCurve, Curve &subdivCurve, int level);

    /**
     * Checks whether v0 and v3 reside in the same half plane with respect to the edge v1-v2.
     * @param v0 Vertex 0.
     * @param v1 Vertex 1.
     * @param v2 Vertex 2.
     * @param v3 Vertex 3.
     * @return True if v0 and v1 are in the same half plane. False otherwise.
     */
    [[nodiscard]] bool areInSameHalfPlane(const Vector2DD &v0,
                                          const Vector2DD &v1,
                                          const Vector2DD &v2,
                                          const Vector2DD &v3) const;

    /**
     * Inserts an edge point/normal in the newPoints/newNormals collection.
     * @param controlCurve The control curve from which to extract patch data to construct the new edge point.
     * @param subdivCurve The curve where the calculated point-normal pair will be inserted.
     * @param i The index of the first vertex of the edge to find the patch for. That is, for the edge A-B, i denotes the index of A. This is the index with respect to the newPoints/nerNormals collection.
     */
    void edgePoint(const Curve &controlCurve, Curve &subdivCurve, int i) const;

    /**
     * For 3 vertices A-B-C, this calculates the normal of the inflection point on the edge B-C based only on the A, B and C.
     * @param edgeAB The edge A-B
     * @param edgeBC The edge B-C.
     * @param orthogonal A vector orthogonal to B-C.
     * @return The inflection point normal and the angle the normal makes with the orthogonal line.
     */
    std::pair<Vector2DD, real_t> inflNormal(const Vector2DD &edgeAB,
                                            const Vector2DD &edgeBC,
                                            const Vector2DD &orthogonal) const;

    void insertInflPoints(const Curve &curve, Curve& targetCurve);
};

} // namespace conis::core