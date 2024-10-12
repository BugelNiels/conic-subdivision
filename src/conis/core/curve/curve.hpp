#pragma once

#include <set>

#include "conis/core/vector.hpp"
#include "conis/core/curve/curvaturetype.hpp"

namespace conis::core {

class Settings;

/**
 * @brief The Curve class is a basic representation of a 2D curve.
 */
class Curve {
public:
    Curve();
    Curve(bool closed);

    Curve(std::vector<Vector2DD> coords, bool closed = true);

    Curve(std::vector<Vector2DD> coords, std::vector<Vector2DD> normals, bool closed = true);

    Curve(const Curve &other) = default;

    Curve &operator=(const Curve &other) = default;

    void copyDataTo(Curve &other) const;

    [[nodiscard]] inline const std::vector<Vector2DD> &getCoords() const { return coords_; }
    [[nodiscard]] inline const std::vector<Vector2DD> &getNormals() const { return normals_; }
    [[nodiscard]] inline const std::vector<bool> &getCustomNormals() const { return customNormals_; }
    [[nodiscard]] inline const Vector2DD &getCoord(int idx) const { return coords_[idx]; }
    [[nodiscard]] inline const Vector2DD &getNormal(int idx) const { return normals_[idx]; }
    [[nodiscard]] inline const bool isCustomNormal(int idx) const { return customNormals_[idx]; }

    [[nodiscard]] inline std::vector<Vector2DD> &getCoords() { return coords_; }
    [[nodiscard]] inline std::vector<Vector2DD> &getNormals() { return normals_; }
    [[nodiscard]] inline Vector2DD &getCoord(int idx) { return coords_[idx]; }
    [[nodiscard]] inline Vector2DD &getNormal(int idx) { return normals_[idx]; }
    [[nodiscard]] inline std::vector<bool> &getCustomNormals() { return customNormals_; }

    inline void setCoords(std::vector<Vector2DD> coords) { coords_ = coords; }
    inline void setNormals(std::vector<Vector2DD> normals) { normals_ = normals; }

    inline void setCoords(int idx, Vector2DD coord) { coords_[idx] = coord; }
    inline void setNormals(int idx, Vector2DD normal) { normals_[idx] = normal; }
    inline void setCustomNormals(std::vector<bool> customNormals) { customNormals_ = customNormals; }

    [[nodiscard]] int findClosestEdge(const Vector2DD &p, double maxDist) const;
    [[nodiscard]] int findClosestVertex(const Vector2DD &p, double maxDist) const;
    [[nodiscard]] int findClosestNormal(const Vector2DD &p, double maxDist, const double normalLength) const;
    [[nodiscard]] int getNextIdx(int idx) const;
    [[nodiscard]] int getPrevIdx(int idx) const;
    [[nodiscard]] bool isClosed() const;
    [[nodiscard]] Vector2DD prevEdge(int idx) const; // In the line segment a-b-c returns bc
    [[nodiscard]] Vector2DD nextEdge(int idx) const; // in the line segment a-b-c returns ba
    int edgePointingDir(int idx) const; // -1 for inside, 0 for flat and +1 for outside
    int vertexPointingDir(int idx) const; // -1 for inside, 0 for flat and +1 for outside

    int addPoint(const Vector2DD &p);
    void setVertexPosition(int idx, const Vector2DD &p);
    void setNormal(int idx, const Vector2DD &normal);
    void removePoint(int idx);

    void recalculateNormals(bool areaWeightedNormals = false, bool circleNormals = false);
    void recalculateNormal(int idx);

    void setClosed(bool closed);
    void translate(const Vector2DD &translation);
    int numPoints() const;
    real_t curvatureAtIdx(int idx, CurvatureType curvatureType) const;

private:
    bool closed_ = true;
    bool areaWeightedNormals_ = true;
    bool circleNormals_ = false;

    std::vector<Vector2DD> coords_;
    std::vector<Vector2DD> normals_;
    std::vector<bool> customNormals_;

    Vector2DD getClosestPointOnLineSegment(const Vector2DD &start, const Vector2DD &end, const Vector2DD &point) const;
    [[nodiscard]] std::vector<Vector2DD> calcNormals(const std::vector<Vector2DD> &coords) const;

    [[nodiscard]] Vector2DD calcNormalAtIndex(const std::vector<Vector2DD> &coords,
                                              const std::vector<Vector2DD> &normals,
                                              int i) const;

    [[nodiscard]] int findInsertIdx(const Vector2DD &p) const;
};

} // namespace conis::core