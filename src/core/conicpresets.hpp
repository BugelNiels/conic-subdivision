#pragma once

#include <QString>
#include <QMap>
#include "subdivisioncurve.hpp"

namespace conics {

    class ConicPresets {

    public:
        ConicPresets();
        ~ConicPresets();

        SubdivisionCurve getPreset(const QString& name) const;
        QList<QString> getPresetNames() const;


    private:
        QMap<QString, SubdivisionCurve> presets;

        SubdivisionCurve getPentagon();

        SubdivisionCurve getBasis();

        SubdivisionCurve getG();

        SubdivisionCurve getCircle(int numPoints, float radius);

        SubdivisionCurve getEllipse(int numPoints, float width, float height);
    };

} // conics
