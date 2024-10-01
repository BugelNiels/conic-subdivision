#pragma once

#include <QString>
#include <map>

#include "core/curve/curve.hpp"

namespace conics::core {

class CurvePresetFactory {

public:
    explicit CurvePresetFactory();

    ~CurvePresetFactory();

    [[nodiscard]] Curve getPreset(const QString &name) const;

    [[nodiscard]] std::vector<QString> getPresetNames() const;

private:
    std::map<QString, std::function<Curve()>> presets_;
};

} // namespace conics::core
