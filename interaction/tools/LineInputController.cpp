#include "interaction/tools/LineInputController.h"

namespace arz::interaction {

void LineInputController::activate() noexcept {
    firstPoint_.reset();
    state_ = LineInputState::AwaitingFirstPoint;
}

void LineInputController::cancel() noexcept {
    firstPoint_.reset();
    state_ = LineInputState::Inactive;
}

std::optional<LineInput> LineInputController::acceptPoint(
    arz::geometry::Point2D point
) noexcept {
    if (state_ == LineInputState::Inactive) {
        return std::nullopt;
    }

    if (state_ == LineInputState::AwaitingFirstPoint) {
        firstPoint_ = point;
        state_ = LineInputState::AwaitingSecondPoint;
        return std::nullopt;
    }

    const LineInput result{*firstPoint_, point};
    firstPoint_.reset();
    state_ = LineInputState::AwaitingFirstPoint;
    return result;
}

LineInputState LineInputController::state() const noexcept {
    return state_;
}

std::optional<arz::geometry::Point2D>
LineInputController::firstPoint() const noexcept {
    return firstPoint_;
}

}
