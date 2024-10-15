#pragma once

#include <map>

#include "conis/core/curve/curve.hpp"

namespace conis::core {

class CurvePresetFactory {

public:
    explicit CurvePresetFactory();

    ~CurvePresetFactory();

    [[nodiscard]] Curve getPreset(const std::string &name) const;

    [[nodiscard]] std::vector<std::string> getPresetNames() const;

private:
    std::map<std::string, std::function<Curve()>> presets_;
};

} // namespace conis::core
