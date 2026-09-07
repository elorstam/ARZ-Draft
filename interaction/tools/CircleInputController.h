#pragma once
#include <optional>
#include "geometry/primitives/Point2D.h"
namespace arz::interaction {
enum class CircleInputState { Inactive = 0, AwaitingCenter, AwaitingRadiusPoint };
class CircleInputController final {
public:
    void activate() noexcept;
    void cancel() noexcept;
    [[nodiscard]] std::optional<double> acceptPoint(arz::geometry::Point2D point) noexcept;
    [[nodiscard]] CircleInputState state() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> center() const noexcept;
private:
    CircleInputState state_{CircleInputState::Inactive};
    std::optional<arz::geometry::Point2D> center_;
};
}
