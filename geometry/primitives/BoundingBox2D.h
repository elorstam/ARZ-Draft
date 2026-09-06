#pragma once

namespace arz::geometry {

struct BoundingBox2D final {
    double minX{0.0};
    double minY{0.0};
    double maxX{0.0};
    double maxY{0.0};

    [[nodiscard]] double width() const noexcept {
        return maxX - minX;
    }

    [[nodiscard]] double height() const noexcept {
        return maxY - minY;
    }

    [[nodiscard]] bool contains(
        double x,
        double y
    ) const noexcept {
        return x >= minX
            && x <= maxX
            && y >= minY
            && y <= maxY;
    }
};

}
