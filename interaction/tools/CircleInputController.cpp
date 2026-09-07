#include "interaction/tools/CircleInputController.h"
#include "geometry/algorithms/Point2DOperations.h"
namespace arz::interaction {
void CircleInputController::activate() noexcept { center_.reset(); state_ = CircleInputState::AwaitingCenter; }
void CircleInputController::cancel() noexcept { center_.reset(); state_ = CircleInputState::Inactive; }
std::optional<double> CircleInputController::acceptPoint(arz::geometry::Point2D point) noexcept {
    if (state_ == CircleInputState::Inactive) return std::nullopt;
    if (state_ == CircleInputState::AwaitingCenter) { center_ = point; state_ = CircleInputState::AwaitingRadiusPoint; return std::nullopt; }
    const double radius = arz::geometry::distance(*center_, point);
    cancel();
    return radius;
}
CircleInputState CircleInputController::state() const noexcept { return state_; }
std::optional<arz::geometry::Point2D> CircleInputController::center() const noexcept { return center_; }
}
