#pragma once

#include <QPalette>
#include <QColor>
#include "ui/renderers/util/colormap.hpp"

class Settings;

namespace conics::ui {

    typedef struct StylePreset {
        QColor backgroundCol;
        QColor controlCurveCol;
        QColor controlPointCol;
        QColor smoothCurveCol;
        QColor selectedVertCol;
        QColor selectedNormCol;
        QColor normCol;
        ColorMapName colorMapName;

        QPalette palette;
    } StylePreset;

    void applyStylePreset(Settings &settings, const StylePreset &preset);

    StylePreset getDarkModePalette();

    StylePreset getLightModePalette();
} // conics
