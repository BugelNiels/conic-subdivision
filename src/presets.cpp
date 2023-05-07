#include "presets.h"

conics::Presets::Presets() {

}

conics::Presets::~Presets() {

}

SubdivisionCurve conics::Presets::getPreset(const QString &name) {
    return presets[name];
}

QList<QString> conics::Presets::getPresetNames() const {
    return presets.keys();
}
