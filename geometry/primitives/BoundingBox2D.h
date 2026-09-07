#pragma once

#include <algorithm>

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

    [[nodiscard]] BoundingBox2D normalized() const noexcept {
        return {
            std::min(minX, maxX),
            std::min(minY, maxY),
            std::max(minX, maxX),
            std::max(minY, maxY)
        };
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

    [[nodiscard]] bool intersects(
        const BoundingBox2D& other
    ) const noexcept {
        const auto first = normalized();
        const auto second = other.normalized();

        return first.minX <= second.maxX
            && first.maxX >= second.minX
            && first.minY <= second.maxY
            && first.maxY >= second.minY;
    }

    [[nodiscard]] bool contains(
        const BoundingBox2D& other
    ) const noexcept {
        const auto outer = normalized();
        const auto inner = other.normalized();

        return inner.minX >= outer.minX
            && inner.maxX <= outer.maxX
            && inner.minY >= outer.minY
            && inner.maxY <= outer.maxY;
    }
};

}
