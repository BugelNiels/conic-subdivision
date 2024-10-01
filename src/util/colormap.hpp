#pragma once

#include <QMap>
#include <QVector3D>

/**
 * @brief The ColorMapName enum contains the different available color maps.
 */
enum class ColorMapName {
    TURBO,
    PLASMA,
    BLACK_BODY,
    INFERNO,
    KINDLMANN,
    SMOOTH_COOL_WARM,
    MPL_COOL,
    OCEAN_DEEP,
    CMP_HAXBY
};

/**
 * @brief The ColorMap class allows you to construct different color maps.
 */
class ColorMap {
public:
    ColorMap();

    std::vector<QVector3D> &getColorMap(ColorMapName name);

    inline int getNumColorMaps() { return int(colorMaps.size()); }

private:
    QMap<ColorMapName, std::vector<QVector3D>> colorMaps;

    std::vector<QVector3D> getTurboRainbowColorMap();

    std::vector<QVector3D> getPlasmaColorMap();

    std::vector<QVector3D> getBlackBodyColorMap();

    std::vector<QVector3D> getInfernoColorMap();

    std::vector<QVector3D> getKindlmannColorMap();

    std::vector<QVector3D> getSmoothCoolWarmColorMap();

    std::vector<QVector3D> getMPLCoolColorMap();

    std::vector<QVector3D> getOceanDeepColorMap();

    std::vector<QVector3D> getCMPHaxbyColorMap();
};