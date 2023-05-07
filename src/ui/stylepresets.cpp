#include "stylepresets.hpp"

#include <QApplication>
#include "settings.hpp"

void conics::ui::applyStylePreset(Settings &settings, const StylePreset &preset) {
    QApplication::setPalette(preset.palette);
    settings.style = preset;
}

conics::ui::StylePreset conics::ui::getDarkModePalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(45, 45, 47));             // Window background color
    palette.setColor(QPalette::WindowText, QColor(239, 239, 239));      // Text color
    palette.setColor(QPalette::Base, QColor(40, 40, 40));               // Base color
    palette.setColor(QPalette::AlternateBase, QColor(45, 45, 45));      // Alternate base color
    palette.setColor(QPalette::ToolTipBase, QColor(239, 239, 239));     // ToolTip base color
    palette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));           // ToolTip text color
    palette.setColor(QPalette::Text, QColor(239, 239, 239));            // Text color
    palette.setColor(QPalette::Button, QColor(70, 70, 72));             // Button color
    palette.setColor(QPalette::ButtonText, QColor(239, 239, 239));      // Button text color
    palette.setColor(QPalette::Highlight, QColor(75, 110, 175));        // Highlight color
    palette.setColor(QPalette::HighlightedText, QColor(239, 239, 239)); // Highlight text color
    palette.setColor(QPalette::PlaceholderText, QColor(150, 150, 150)); // PlaceHolder text color

    StylePreset preset;
    preset.palette = palette;

    preset.backgroundCol = QColor(20, 20, 20);
    preset.controlCurveCol = QColor(255, 255, 255);
    preset.controlPointCol = QColor(255, 150, 150);
    preset.smoothCurveCol = QColor(50, 50, 255);
    preset.selectedCol = QColor(255, 0, 0);
    preset.normCol = QColor(150, 150, 255);
    return preset;
}

conics::ui::StylePreset conics::ui::getLightModePalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(222, 222, 222));           // Window background color
    palette.setColor(QPalette::WindowText, QColor(70, 70, 70));          // Text color
    palette.setColor(QPalette::Base, QColor(240, 240, 240));             // Base color
    palette.setColor(QPalette::AlternateBase, QColor(250, 250, 250));    // Alternate base color
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 225));      // ToolTip base color
    palette.setColor(QPalette::ToolTipText, QColor(70, 70, 70));         // ToolTip text color
    palette.setColor(QPalette::Text, QColor(70, 70, 70));                // Text color
    palette.setColor(QPalette::Button, QColor(220, 220, 220));           // Button color
    palette.setColor(QPalette::ButtonText, QColor(70, 70, 70));          // Button text color
    palette.setColor(QPalette::Highlight, QColor(75, 110, 175));         // Highlight color
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));  // Highlight text color
    palette.setColor(QPalette::PlaceholderText, QColor(170, 170, 170));  // Placeholder text color

    StylePreset preset;
    preset.palette = palette;

    preset.backgroundCol = QColor(255, 255, 255);
    preset.controlCurveCol = QColor(70, 70, 70);
    preset.controlPointCol = QColor(255, 150, 150);
    preset.smoothCurveCol = QColor(50, 50, 255);
    preset.selectedCol = QColor(255, 0, 0);
    preset.normCol = QColor(150, 150, 255);
    return preset;
}