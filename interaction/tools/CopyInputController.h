#pragma once

#include <optional>

#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

enum class CopyInputState { Inactive = 0, AwaitingBasePoint, AwaitingDestination };

class CopyInputController final {
public:
    void activate() noexcept;
    void cancel() noexcept;
    void acceptBasePoint(arz::geometry::Point2D point) noexcept;
    [[nodiscard]] CopyInputState state() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> basePoint() const noexcept;

private:
    CopyInputState state_{CopyInputState::Inactive};
    std::optional<arz::geometry::Point2D> basePoint_;
};

}
