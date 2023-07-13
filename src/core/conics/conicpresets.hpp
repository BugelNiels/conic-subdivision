#pragma once

#include <QString>
#include <QMap>
#include "src/core/subdivisioncurve.hpp"

namespace conics {

    class ConicPresets {

    public:
        explicit ConicPresets(const Settings &settings);

        ~ConicPresets();

        [[nodiscard]] SubdivisionCurve getPreset(const QString &name) const;

        [[nodiscard]] std::vector<QString> getPresetNames() const;

    private:
        const Settings &settings_;

        std::map<QString, std::shared_ptr<SubdivisionCurve>> presets_;

        SubdivisionCurve getPentagon();

        SubdivisionCurve getBasis();

        SubdivisionCurve getG();

        SubdivisionCurve getCircle(int numPoints, double radius);

        SubdivisionCurve getEllipse(int numPoints, double width, double height);

        SubdivisionCurve getBlank();

        SubdivisionCurve getLine();

        SubdivisionCurve getStair();

        SubdivisionCurve getStar();
    };

} // conics
