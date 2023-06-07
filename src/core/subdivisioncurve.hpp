#pragma once

#include <QString>
#include <QSet>
#include "util/vector.hpp"
#include "core/subdivision/conicsubdivider.hpp"

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

    inline const std::vector<Vector2DD>& getNetCoords() const { return netCoords_; }

    inline const std::vector<Vector2DD>& getNetNormals() const { return netNormals_; }

    inline const std::vector<Vector2DD>& getCurveCoords() const { return curveCoords_; }

    inline const std::vector<Vector2DD>& getCurveNormals() const { return curveNormals_; }

    inline int getSubdivLevel() const { return subdivisionLevel_; }

    int findClosestVertex(const Vector2DD &p, double maxDist) const;

    int findClosestNormal(const Vector2DD &p, double maxDist) const;

    int addPoint(const Vector2DD &p);

    void setVertexPosition(int idx, const Vector2DD &p);

    void setNormalPosition(int idx, const Vector2DD &p);

    void removePoint(int idx);

    void subdivide(int level);

    void reSubdivide();

    void recalculateNormals();

    void recalculateNormal(int idx);

    bool isClosed() const;

    void setClosed(bool closed);

    void insertKnots();

    void applySubdivision();

    std::vector<double> getStabilityVals() const;

    void translate(const Vector2DD &translation);

private:
    const Settings &settings_;
    ConicSubdivider subdivider;

    int subdivisionLevel_ = 0;
    bool closed_ = false;

    std::vector<Vector2DD> curveCoords_;
    std::vector<Vector2DD> curveNormals_;
    std::vector<bool> customNormals_;
    QSet<int> knotIndices_;

    std::vector<Vector2DD> netCoords_;
    std::vector<Vector2DD> netNormals_;


    std::vector<Vector2DD> calcNormals(const std::vector<Vector2DD> &coords) const;

    Vector2DD calcNormalAtIndex(const std::vector<Vector2DD> &coords, const std::vector<Vector2DD> &normals, int i) const;

    int findInsertIdx(const Vector2DD &p) const;

    int getNextIdx(int idx) const;

    int getPrevIdx(int idx) const;

    friend class ConicSubdivider;

};
