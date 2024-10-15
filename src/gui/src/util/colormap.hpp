#pragma once

#include <QMap>
#include <QVector3D>

namespace conis::gui {

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

    int getNumColorMaps() const { return static_cast<int>(colorMaps.size()); }

private:
    QMap<ColorMapName, std::vector<QVector3D>> colorMaps;

    static std::vector<QVector3D> getTurboRainbowColorMap();

    static std::vector<QVector3D> getPlasmaColorMap();

    static std::vector<QVector3D> getBlackBodyColorMap();

    static std::vector<QVector3D> getInfernoColorMap();

    static std::vector<QVector3D> getKindlmannColorMap();

    static std::vector<QVector3D> getSmoothCoolWarmColorMap();

    static std::vector<QVector3D> getMPLCoolColorMap();

    static std::vector<QVector3D> getOceanDeepColorMap();

    static std::vector<QVector3D> getCMPHaxbyColorMap();
};

} // namespace conis::gui
