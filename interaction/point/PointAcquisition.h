#pragma once

#include <optional>

#include "cad/snapping/SnapResult.h"
#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

struct PointAcquisitionContext final {
    arz::geometry::Point2D rawPoint{};
    std::optional<arz::cad::SnapResult> snap;
    std::optional<arz::geometry::Point2D> constraintOrigin;
    bool orthoEnabled{false};
};

struct ResolvedCadPoint final {
    arz::geometry::Point2D rawPoint{};
    std::optional<arz::geometry::Point2D> snappedPoint;
    std::optional<arz::geometry::Point2D> constrainedPoint;
    arz::geometry::Point2D point{};
    std::optional<arz::cad::SnapResult> snap;
    bool orthoConstrained{false};
};

class PointAcquisition final {
public:
    [[nodiscard]] ResolvedCadPoint resolve(
        const PointAcquisitionContext& context
    ) const noexcept;
};

}
