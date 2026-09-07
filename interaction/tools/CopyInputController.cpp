#include "interaction/tools/CopyInputController.h"

namespace arz::interaction {

void CopyInputController::activate() noexcept {
    basePoint_.reset();
    state_ = CopyInputState::AwaitingBasePoint;
}

void CopyInputController::cancel() noexcept {
    basePoint_.reset();
    state_ = CopyInputState::Inactive;
}

void CopyInputController::acceptBasePoint(arz::geometry::Point2D point) noexcept {
    basePoint_ = point;
    state_ = CopyInputState::AwaitingDestination;
}

CopyInputState CopyInputController::state() const noexcept { return state_; }

std::optional<arz::geometry::Point2D> CopyInputController::basePoint() const noexcept {
    return basePoint_;
}

}
