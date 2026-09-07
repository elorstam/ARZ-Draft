#include "interaction/tools/PolylineInputController.h"
namespace arz::interaction {
void PolylineInputController::activate() noexcept { vertices_.clear(); state_ = PolylineInputState::AwaitingStartPoint; }
void PolylineInputController::cancel() noexcept { vertices_.clear(); state_ = PolylineInputState::Inactive; }
bool PolylineInputController::acceptPoint(arz::geometry::Point2D point) {
    if (state_ == PolylineInputState::Inactive) return false;
    vertices_.push_back(point);
    state_ = PolylineInputState::AwaitingNextPoint;
    return true;
}
PolylineInputState PolylineInputController::state() const noexcept { return state_; }
const std::vector<arz::geometry::Point2D>& PolylineInputController::vertices() const noexcept { return vertices_; }
bool PolylineInputController::canFinish() const noexcept { return vertices_.size() >= 2; }
bool PolylineInputController::canClose() const noexcept { return vertices_.size() >= 3; }
}
