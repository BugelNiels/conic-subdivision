#pragma once

#include "gui/renderers/util/colormap.hpp"
#include <QColor>
#include <QPalette>

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
} // namespace conics::ui
