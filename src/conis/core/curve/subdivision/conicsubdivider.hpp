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
     * Inserts inflection points such that the curve can be split into globally convex segments.
     * @param curve The curve that stores the information on the inflection point indices.
     * @param coords Collection where the coordinates of the curve + inflection point coordinates will be put into.
     * @param norms Collection where the normals of the curve + inflection point normals will be put into.
     * @return Collection that stores for each normal whether it should be recalculated or not.
     * The inflection points have a special normal, so these will have the value true at their indices.
     */
    std::vector<bool> getInflPointCurve(const Curve &curve,
                                        std::vector<Vector2DD> &coords,
                                        std::vector<Vector2DD> &normals);

    /**
     * Extracts a patch of a given neighbourhood from a curve. Maintains convexity provided that inflection points were inserted.
     * @param points The coordinates of the curve.
     * @param normals The normals of the curve.
     * @param i The index of the first vertex of the edge to find the patch for. That is, for the edge A-B, i denotes the index of A.
     * @param maxPatchSize The maximum number of points allowed in the patch on either side of the edge.
     * The total maximum size of the patch will be 2 * maxPatchSize.
     * For example, maxPatchSize = 1 means only the edge points are included, while maxPatchSize = 2 means a total maximum size of 4.
     * @param closed Whether the curve is closed or not.
     * @return A collection of patch points.
     */
    std::vector<PatchPoint> extractPatch(const std::vector<Vector2DD> &points,
                                         const std::vector<Vector2DD> &normals,
                                         int pIdx,
                                         int maxPatchSize,
                                         bool closed) const;

private:
    const SubdivisionSettings &settings_;
    std::vector<int> inflPointIndices_;
    // These buffers persist between between subdivisions to prevent re-allocation
    std::vector<Vector2DD> pointsBuffer;
    std::vector<Vector2DD> normalsBuffer;

    void subdivideRecursive(std::vector<Vector2DD> &points,
                            std::vector<Vector2DD> &normals,
                            std::vector<Vector2DD> &newPoints,
                            std::vector<Vector2DD> &newNormals,
                            int numPoints,
                            int level,
                            bool closed);

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
     * @param points The coordinates at subdivision level d.
     * @param normals The normals at subdivision level d.
     * @param newPoints The coordinates at subdivision level d+1.
     * @param newNormals The normals at subdivision level d+1.
     * @param i The index of the first vertex of the edge to find the patch for. That is, for the edge A-B, i denotes the index of A. This is the index with respect to the newPoints/nerNormals collection.
     * @param closed Whether the curve is closed or not.
     */
    void edgePoint(const std::vector<Vector2DD> &points,
                   const std::vector<Vector2DD> &normals,
                   std::vector<Vector2DD> &newPoints,
                   std::vector<Vector2DD> &newNormals,
                   int i,
                   bool closed) const;

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
};

} // namespace conis::core