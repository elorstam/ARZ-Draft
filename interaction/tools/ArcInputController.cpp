#include "interaction/tools/ArcInputController.h"
#include <cmath>
#include "geometry/algorithms/Curve2D.h"
namespace arz::interaction {
namespace {
std::optional<ThreePointArcInput> construct(arz::geometry::Point2D first,
    arz::geometry::Point2D second, arz::geometry::Point2D end) noexcept {
    const auto circle = arz::geometry::circumcircle(first, second, end);
    if (!circle) return std::nullopt;
    const double startAngle = std::atan2(first.y - circle->center.y, first.x - circle->center.x);
    const double secondAngle = std::atan2(second.y - circle->center.y, second.x - circle->center.x);
    const double endAngle = std::atan2(end.y - circle->center.y, end.x - circle->center.x);
    const bool ccw = arz::geometry::angleOnArc(secondAngle, startAngle, endAngle, true);
    return ThreePointArcInput{circle->center, circle->radius,
        arz::geometry::normalizeAngle(startAngle), arz::geometry::normalizeAngle(endAngle), ccw};
}
}
void ArcInputController::activate() noexcept { start_.reset(); second_.reset(); state_ = ArcInputState::AwaitingStart; }
void ArcInputController::cancel() noexcept { start_.reset(); second_.reset(); state_ = ArcInputState::Inactive; }
std::optional<ThreePointArcInput> ArcInputController::acceptPoint(arz::geometry::Point2D point) noexcept {
    if (state_ == ArcInputState::Inactive) return std::nullopt;
    if (state_ == ArcInputState::AwaitingStart) { start_ = point; state_ = ArcInputState::AwaitingSecond; return std::nullopt; }
    if (state_ == ArcInputState::AwaitingSecond) { second_ = point; state_ = ArcInputState::AwaitingEnd; return std::nullopt; }
    const auto result = construct(*start_, *second_, point);
    if (result) cancel();
    return result;
}
ArcInputState ArcInputController::state() const noexcept { return state_; }
std::optional<arz::geometry::Point2D> ArcInputController::start() const noexcept { return start_; }
std::optional<arz::geometry::Point2D> ArcInputController::second() const noexcept { return second_; }
std::optional<ThreePointArcInput> ArcInputController::preview(arz::geometry::Point2D end) const noexcept {
    return start_ && second_ ? construct(*start_, *second_, end) : std::nullopt;
}
}
