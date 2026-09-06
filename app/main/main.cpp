#include <iostream>

#include "core/units/Length.h"
#include "geometry/primitives/Point2D.h"

int main() {
    const auto wallThickness =
        arz::core::Length::fromCentimeters(20.0);

    const arz::geometry::Point2D origin{
        0.0,
        0.0
    };

    std::cout
        << "ARZ Draft\n"
        << "Version: 0.1.0\n"
        << "Wall thickness: "
        << wallThickness.millimeters()
        << " mm\n"
        << "Origin: "
        << origin.x << ", "
        << origin.y
        << '\n';

    return 0;
}
