#pragma once

#include <QMap>
#include <QVector3D>
#include <QVector>

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

    QVector<QVector3D> &getColorMap(ColorMapName name);

    inline int getNumColorMaps() { return colorMaps.size(); }

private:
    QMap<ColorMapName, QVector<QVector3D>> colorMaps;

    QVector<QVector3D> getTurboRainbowColorMap();

    QVector<QVector3D> getPlasmaColorMap();

    QVector<QVector3D> getBlackBodyColorMap();

    QVector<QVector3D> getInfernoColorMap();

    QVector<QVector3D> getKindlmannColorMap();

    QVector<QVector3D> getSmoothCoolWarmColorMap();

    QVector<QVector3D> getMPLCoolColorMap();

    QVector<QVector3D> getOceanDeepColorMap();

    QVector<QVector3D> getCMPHaxbyColorMap();
};