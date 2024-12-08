#pragma once

#include <set>
#include <utility>

#include "conis/core/curve/curvaturetype.hpp"
#include "conis/core/vector.hpp"

namespace conis::core {

class Settings;

/**
 * @brief The Curve class is a basic representation of a 2D curve.
 */
class Curve {
public:
    Curve();
    explicit Curve(bool closed);

    explicit Curve(std::vector<Vector2DD> verts, bool closed = true);

    Curve(std::vector<Vector2DD> verts,
          std::vector<Vector2DD> normals,
          bool closed = true);

    Curve(const Curve &other) = default;

    Curve &operator=(const Curve &other) = default;

    void copyDataTo(Curve &other) const;

    // TODO: don't define these in the header files and add additional checks to them
    [[nodiscard]] const std::vector<Vector2DD> &getVertices() const {
        return vertices_;
    }
    [[nodiscard]] const std::vector<Vector2DD> &getNormals() const {
        return normals_;
    }
    [[nodiscard]] const std::vector<bool> &getCustomNormals() const { return customNormals_; }
    [[nodiscard]] const Vector2DD &getVertex(const int idx) const { return vertices_[idx]; }
    [[nodiscard]] const Vector2DD &getNormal(const int idx) const { return normals_[idx]; }
    [[nodiscard]] bool isCustomNormal(const int idx) const { return customNormals_[idx]; }

    [[nodiscard]] std::vector<Vector2DD> &getVertices() { return vertices_; }
    [[nodiscard]] std::vector<Vector2DD> &getNormals() { return normals_; }
    [[nodiscard]] Vector2DD &getVertex(const int idx) { return vertices_[idx]; }
    [[nodiscard]] Vector2DD &getNormal(const int idx) { return normals_[idx]; }
    [[nodiscard]] std::vector<bool> &getCustomNormals() { return customNormals_; }

    void setCoords(std::vector<Vector2DD> verts) { vertices_ = std::move(verts); }
    void setNormals(std::vector<Vector2DD> normals) {
        normals_ = std::move(normals);
    }

    void setVertex(int idx, Vector2DD coord);
    void setNormal(int idx, Vector2DD normal);
    void setCustomNormals(std::vector<bool> customNormals) { customNormals_ = std::move(customNormals); }

    [[nodiscard]] int findClosestEdge(const Vector2DD &p, double maxDist) const;
    [[nodiscard]] int findClosestVertex(const Vector2DD &p, double maxDist) const;
    [[nodiscard]] int findClosestNormal(const Vector2DD &p, double maxDist, double normalLength) const;
    [[nodiscard]] int getNextIdx(int idx) const;
    [[nodiscard]] int getPrevIdx(int idx) const;
    [[nodiscard]] bool isClosed() const;
    [[nodiscard]] Vector2DD prevEdge(int idx) const; // In the line segment a-b-c returns bc
    [[nodiscard]] Vector2DD nextEdge(int idx) const; // in the line segment a-b-c returns ba
    int edgePointingDir(int idx) const;              // -1 for inside, 0 for flat and +1 for outside
    int vertexPointingDir(int idx) const;            // -1 for inside, 0 for flat and +1 for outside

    int addPoint(const Vector2DD &p);
    void setVertexPosition(int idx, const Vector2DD &p);
    void setCustomNormal(int idx, const Vector2DD &normal);
    void removePoint(int idx);

    void recalculateNormals(bool areaWeightedNormals = false, bool circleNormals = false);
    void recalculateNormal(int idx);

    void setClosed(bool closed, bool recalculate = true);
    void translate(const Vector2DD &translation);
    int numPoints() const;
    real_t curvatureAtIdx(int idx, CurvatureType curvatureType) const;

private:
    bool closed_ = true;
    bool areaWeightedNormals_ = true;
    bool circleNormals_ = false;

    std::vector<Vector2DD> vertices_;
    std::vector<Vector2DD> normals_;
    std::vector<bool> customNormals_;

    Vector2DD getClosestPointOnLineSegment(const Vector2DD &start, const Vector2DD &end, const Vector2DD &point) const;
    [[nodiscard]] std::vector<Vector2DD> calcNormals(const std::vector<Vector2DD> &verts) const;

    [[nodiscard]] Vector2DD calcNormalAtIndex(const std::vector<Vector2DD> &verts, int i) const;

    [[nodiscard]] int findInsertIdx(const Vector2DD &p) const;
};

} // namespace conis::core