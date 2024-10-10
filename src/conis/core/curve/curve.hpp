#pragma once

#include <set>

#include "conis/core/vector.hpp"

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

    void copyDataTo(Curve& other) const;

    [[nodiscard]] inline const std::vector<Vector2DD> &getCoords() const { return coords_; }
    [[nodiscard]] inline const std::vector<Vector2DD> &getNormals() const { return normals_; }
    [[nodiscard]] inline const std::vector<bool> &getCustomNormals() const { return customNormals_; }

    [[nodiscard]] inline std::vector<Vector2DD> &getCoords() { return coords_; }
    [[nodiscard]] inline std::vector<Vector2DD> &getNormals() { return normals_; }
    [[nodiscard]] inline std::vector<bool> &getCustomNormals() { return customNormals_; }

    inline void setCoords(std::vector<Vector2DD> coords) { coords_ = coords; }
    inline void setNormals(std::vector<Vector2DD> normals) { normals_ = normals; }
    inline void setCustomNormals(std::vector<bool> customNormals) { customNormals_ = customNormals; }

    [[nodiscard]] int findClosestVertex(const Vector2DD &p, double maxDist) const;

    [[nodiscard]] int findClosestNormal(const Vector2DD &p, double maxDist, const double normalLength) const;

    [[nodiscard]] bool isClosed() const;

    int addPoint(const Vector2DD &p);

    void setVertexPosition(int idx, const Vector2DD &p);

    void redirectNormalToPoint(int idx, const Vector2DD &p);

    void removePoint(int idx);

    void recalculateNormals(bool areaWeightedNormals = false, bool circleNormals = false);

    void recalculateNormal(int idx);

    void setClosed(bool closed);

    void translate(const Vector2DD &translation);

    int numPoints() const;

    [[nodiscard]] int getNextIdx(int idx) const;

    [[nodiscard]] int getPrevIdx(int idx) const;

private:
    bool closed_ = true;
    bool areaWeightedNormals_ = true;
    bool circleNormals_ = false;

    std::vector<Vector2DD> coords_;
    std::vector<Vector2DD> normals_;
    std::vector<bool> customNormals_;

    [[nodiscard]] std::vector<Vector2DD> calcNormals(const std::vector<Vector2DD> &coords) const;

    [[nodiscard]] Vector2DD calcNormalAtIndex(const std::vector<Vector2DD> &coords,
                                              const std::vector<Vector2DD> &normals,
                                              int i) const;

    [[nodiscard]] int findInsertIdx(const Vector2DD &p) const;

    Vector2DD calcNormal(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c, bool areaWeighted) const;
};

} // namespace conis::core