#include "core/units/Length.h"

namespace arz::core {

Length::Length(double millimeters)
    : millimeters_(millimeters) {
}

Length Length::fromMillimeters(double value) {
    return Length(value);
}

Length Length::fromCentimeters(double value) {
    return Length(value * 10.0);
}

Length Length::fromMeters(double value) {
    return Length(value * 1000.0);
}

double Length::millimeters() const {
    return millimeters_;
}

double Length::centimeters() const {
    return millimeters_ / 10.0;
}

double Length::meters() const {
    return millimeters_ / 1000.0;
}

}
