#pragma once

#include <optional>
#include <vector>

#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

enum class LineInputState {
    Inactive = 0,
    AwaitingFirstPoint,
    AwaitingSecondPoint
};

struct LineInput final {
    arz::geometry::Point2D start{};
    arz::geometry::Point2D end{};
};

class LineInputController final {
public:
    void activate() noexcept;
    void cancel() noexcept;
    [[nodiscard]] std::optional<LineInput> acceptPoint(
        arz::geometry::Point2D point
    ) noexcept;
    [[nodiscard]] LineInputState state() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D>
    firstPoint() const noexcept;
    [[nodiscard]] bool canUndo() const noexcept;
    [[nodiscard]] bool undoLastSegment() noexcept;

private:
    LineInputState state_{LineInputState::Inactive};
    std::optional<arz::geometry::Point2D> firstPoint_;
    std::vector<arz::geometry::Point2D> chainPoints_;
};

}
