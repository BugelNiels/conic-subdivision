#include "subdivisioncurve.hpp"

#include <utility>

#include "settings/settings.hpp"

SubdivisionCurve::SubdivisionCurve(const Settings &settings, Curve &controlCurve, std::shared_ptr<Subdivider> subdivider)
    : settings_(settings),
      controlCurve_(controlCurve),
      subdivider_(subdivider) {}


void SubdivisionCurve::reSubdivide() {
    subdivide(subdivisionLevel_);
}

void SubdivisionCurve::subdivide(int level) {
    subdividedCurve_ = Curve(controlCurve_.getCoords(), controlCurve_.getNormals(), controlCurve_.isClosed());
    subdivider_->subdivide(subdividedCurve_, level);
    subdivisionLevel_ = level;
}
