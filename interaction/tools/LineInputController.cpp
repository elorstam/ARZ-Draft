#include "interaction/tools/LineInputController.h"

namespace arz::interaction {

void LineInputController::activate() noexcept {
    firstPoint_.reset();
    chainPoints_.clear();
    state_ = LineInputState::AwaitingFirstPoint;
}

void LineInputController::cancel() noexcept {
    firstPoint_.reset();
    chainPoints_.clear();
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
        chainPoints_.push_back(point);
        state_ = LineInputState::AwaitingSecondPoint;
        return std::nullopt;
    }

    const LineInput result{*firstPoint_, point};
    firstPoint_ = point;
    chainPoints_.push_back(point);
    return result;
}

LineInputState LineInputController::state() const noexcept {
    return state_;
}

std::optional<arz::geometry::Point2D>
LineInputController::firstPoint() const noexcept {
    return firstPoint_;
}

bool LineInputController::canUndo() const noexcept {
    return chainPoints_.size() > 1;
}

bool LineInputController::undoLastSegment() noexcept {
    if (!canUndo()) return false;
    chainPoints_.pop_back();
    firstPoint_ = chainPoints_.back();
    state_ = LineInputState::AwaitingSecondPoint;
    return true;
}

}
