#pragma once

#include <QColor>
#include <QPalette>

#include "util/colormap.hpp"

namespace conics::gui {

class ViewSettings;

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

void applyStylePreset(ViewSettings &settings, const StylePreset &preset);

StylePreset getDarkModePalette();

StylePreset getLightModePalette();

} // namespace conics::gui
