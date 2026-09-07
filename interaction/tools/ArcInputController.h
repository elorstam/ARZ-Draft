#pragma once
#include <optional>
#include "geometry/primitives/Point2D.h"
namespace arz::interaction {
enum class ArcInputState { Inactive = 0, AwaitingStart, AwaitingSecond, AwaitingThirdPoint };
struct ThreePointArcInput final {
    arz::geometry::Point2D center{};
    double radius{};
    double startAngle{};
    double endAngle{};
    bool counterClockwise{true};
};
class ArcInputController final {
public:
    void activate() noexcept;
    void cancel() noexcept;
    [[nodiscard]] std::optional<ThreePointArcInput> acceptPoint(arz::geometry::Point2D point) noexcept;
    [[nodiscard]] ArcInputState state() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> start() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> second() const noexcept;
    [[nodiscard]] std::optional<ThreePointArcInput> preview(arz::geometry::Point2D end) const noexcept;
private:
    ArcInputState state_{ArcInputState::Inactive};
    std::optional<arz::geometry::Point2D> start_;
    std::optional<arz::geometry::Point2D> second_;
};
}
