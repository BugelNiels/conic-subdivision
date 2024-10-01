#pragma once

#include <map>
#include <QString>

#include "core/curve.hpp"

namespace conics {

class ConicPresetFactory {

public:
    explicit ConicPresetFactory();

    ~ConicPresetFactory();

    [[nodiscard]] Curve getPreset(const QString &name) const;

    [[nodiscard]] std::vector<QString> getPresetNames() const;

private:

    std::map<QString, std::function<Curve()>> presets_;
};

} // namespace conics
