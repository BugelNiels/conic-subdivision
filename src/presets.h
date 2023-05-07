#pragma once

#include <QString>
#include <QMap>
#include "subdivisioncurve.h"

namespace conics {

    class Presets {

    public:
        Presets();
        ~Presets();

        SubdivisionCurve getPreset(const QString& name);
        QList<QString> getPresetNames() const;


    private:
        QMap<QString, SubdivisionCurve> presets;

    };

} // conics
