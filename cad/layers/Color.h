#pragma once

#include <cstdint>

namespace arz::cad {

struct Color final {
    std::uint8_t red{255};
    std::uint8_t green{255};
    std::uint8_t blue{255};

    [[nodiscard]] bool operator==(const Color&) const = default;

    [[nodiscard]] static constexpr Color white() noexcept {
        return {255, 255, 255};
    }

    [[nodiscard]] static constexpr Color black() noexcept {
        return {0, 0, 0};
    }

    [[nodiscard]] static constexpr Color redColor() noexcept {
        return {255, 0, 0};
    }

    [[nodiscard]] static constexpr Color yellowColor() noexcept {
        return {255, 255, 0};
    }

    [[nodiscard]] static constexpr Color greenColor() noexcept {
        return {0, 255, 0};
    }

    [[nodiscard]] static constexpr Color cyanColor() noexcept {
        return {0, 255, 255};
    }

    [[nodiscard]] static constexpr Color blueColor() noexcept {
        return {0, 0, 255};
    }

    [[nodiscard]] static constexpr Color magentaColor() noexcept {
        return {255, 0, 255};
    }
};

}
