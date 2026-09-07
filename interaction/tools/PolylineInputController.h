#pragma once

#include <vector>

#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

enum class PolylineInputState { Inactive = 0, AwaitingStartPoint, AwaitingNextPoint };

class PolylineInputController final {
public:
    void activate() noexcept;
    void cancel() noexcept;
    [[nodiscard]] bool acceptPoint(arz::geometry::Point2D point);
    [[nodiscard]] PolylineInputState state() const noexcept;
    [[nodiscard]] const std::vector<arz::geometry::Point2D>& vertices() const noexcept;
    [[nodiscard]] bool canFinish() const noexcept;
    [[nodiscard]] bool canClose() const noexcept;
private:
    PolylineInputState state_{PolylineInputState::Inactive};
    std::vector<arz::geometry::Point2D> vertices_;
};

}
