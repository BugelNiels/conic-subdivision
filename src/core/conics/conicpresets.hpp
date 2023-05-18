#pragma once

#include <QString>
#include <QMap>
#include "src/core/subdivisioncurve.hpp"

namespace conics {

    class ConicPresets {

    public:
        explicit ConicPresets(const Settings &settings);

        ~ConicPresets();

        SubdivisionCurve getPreset(const QString &name) const;

        QList<QString> getPresetNames() const;

    private:
        const Settings &settings_;

        QMap<QString, std::shared_ptr<SubdivisionCurve>> presets_;

        SubdivisionCurve getPentagon();

        SubdivisionCurve getBasis();

        SubdivisionCurve getG();

        SubdivisionCurve getCircle(int numPoints, float radius);

        SubdivisionCurve getEllipse(int numPoints, float width, float height);

        SubdivisionCurve getBlank();

        SubdivisionCurve getLine();

        SubdivisionCurve getStair();

        SubdivisionCurve getStar();
    };

} // conics
