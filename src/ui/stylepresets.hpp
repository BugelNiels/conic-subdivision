#pragma once

#include <QPalette>
#include <QColor>

class Settings;

namespace conics::ui {

    typedef struct StylePreset {
        QColor backgroundCol;
        QColor controlCurveCol;
        QColor controlPointCol;
        QColor smoothCurveCol;
        QColor selectedCol;
        QColor normCol;

        QPalette palette;
    } StylePreset;

    void applyStylePreset(Settings& settings, const StylePreset& preset);

    StylePreset getDarkModePalette();

    StylePreset getLightModePalette();
} // conics
