#pragma once

namespace arz::geometry {

struct Point2D final {
    double x{0.0};
    double y{0.0};

    [[nodiscard]] bool operator==(const Point2D& other) const = default;
};

}
