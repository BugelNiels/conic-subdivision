#pragma once

#include "util/colormap.hpp"
#include <QColor>
#include <QPalette>

class Settings;

namespace conics::ui {

using StylePreset = struct StylePreset {
    QColor backgroundCol;
    QColor controlCurveCol;
    QColor controlPointCol;
    QColor smoothCurveCol;
    QColor selectedVertCol;
    QColor selectedNormCol;
    QColor normCol;
    ColorMapName colorMapName;

    QPalette palette;
};

void applyStylePreset(Settings &settings, const StylePreset &preset);

StylePreset getDarkModePalette();

StylePreset getLightModePalette();
} // namespace conics::ui
