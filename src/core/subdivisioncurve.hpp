#pragma once

#include <QString>
#include <set>
#include "util/vector.hpp"
#include "core/subdivision/conicsubdivider.hpp"
#include "core/conics/conic.hpp"

class Settings;

/**
 * @brief The SubdivisionCurve class contains the data of a 2D subdivision
 * curve.
 */
class SubdivisionCurve {
public:

    explicit SubdivisionCurve(const Settings &settings);

    explicit SubdivisionCurve(const Settings &settings, std::vector<Vector2DD> coords, bool closed = true);

    SubdivisionCurve(const Settings &settings, std::vector<Vector2DD> coords, std::vector<Vector2DD> normals,
                     bool closed = true);

    [[nodiscard]] inline const std::vector<Vector2DD> &getNetCoords() const { return netCoords_; }

    [[nodiscard]] inline const std::vector<Vector2DD> &getNetNormals() const { return netNormals_; }

    [[nodiscard]] inline const std::vector<Vector2DD> &getCurveCoords() const { return curveCoords_; }

    [[nodiscard]] inline const std::vector<Vector2DD> &getCurveNormals() const { return curveNormals_; }

    [[nodiscard]] inline int getSubdivLevel() const { return subdivisionLevel_; }

    [[nodiscard]] int findClosestVertex(const Vector2DD &p, double maxDist) const;

    [[nodiscard]] int findClosestNormal(const Vector2DD &p, double maxDist) const;

    [[nodiscard]] bool isClosed() const;

    int addPoint(const Vector2DD &p);

    void setVertexPosition(int idx, const Vector2DD &p);

    void setNormalPosition(int idx, const Vector2DD &p);

    void removePoint(int idx);

    void subdivide(int level);

    void reSubdivide();

    void recalculateNormals();

    void recalculateNormal(int idx);

    Conic getConicAtIndex(int idx);

    void setClosed(bool closed);

    void insertKnots();

    void applySubdivision();

    void translate(const Vector2DD &translation);

    int numPoints() const;

private:
    const Settings &settings_;
    ConicSubdivider subdivider;

    int subdivisionLevel_ = 0;
    bool closed_ = false;

    std::vector<Vector2DD> curveCoords_;
    std::vector<Vector2DD> curveNormals_;
    std::vector<bool> customNormals_;
    std::set<int> knotIndices_;

    std::vector<Vector2DD> netCoords_;
    std::vector<Vector2DD> netNormals_;


    [[nodiscard]] std::vector<Vector2DD> calcNormals(const std::vector<Vector2DD> &coords) const;

    [[nodiscard]] Vector2DD calcNormalAtIndex(const std::vector<Vector2DD> &coords,
                                              const std::vector<Vector2DD> &normals, int i) const;

    [[nodiscard]] int findInsertIdx(const Vector2DD &p) const;

    [[nodiscard]] int getNextIdx(int idx) const;

    [[nodiscard]] int getPrevIdx(int idx) const;

    friend class ConicSubdivider;

    Vector2DD calcNormal(const Vector2DD &a, const Vector2DD &b, const Vector2DD &c, bool areaWeighted) const;
};
