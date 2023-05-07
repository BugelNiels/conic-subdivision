#pragma once

#include <QString>
#include <QMap>
#include "src/core/subdivisioncurve.hpp"

namespace conics {

    class ConicPresets {

    public:
        ConicPresets(Settings *settings);
        ~ConicPresets();

        SubdivisionCurve getPreset(const QString& name) const;
        QList<QString> getPresetNames() const;


    private:
        Settings *settings_;
        QMap<QString, SubdivisionCurve> presets_;

        SubdivisionCurve getPentagon();

        SubdivisionCurve getBasis();

        SubdivisionCurve getG();

        SubdivisionCurve getCircle(int numPoints, float radius);

        SubdivisionCurve getEllipse(int numPoints, float width, float height);
    };

} // conics
