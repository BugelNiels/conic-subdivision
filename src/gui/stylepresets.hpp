#pragma once

#include <QColor>
#include <QPalette>

#include "util/colormap.hpp"

namespace conics::core {
class Settings;
} // namespace conics::core

namespace conics::ui {

using StylePreset = struct StylePreset {
    QColor backgroundCol;
    QColor controlCurveCol;
    QColor controlPointCol;
    QColor smoothCurveCol;
    QColor selectedVertCol;
    QColor selectedNormCol;
    QColor normCol;
    conics::util::ColorMapName colorMapName;

    QPalette palette;
};

void applyStylePreset(conics::core::Settings &settings, const StylePreset &preset);

StylePreset getDarkModePalette();

StylePreset getLightModePalette();

} // namespace conics::ui
