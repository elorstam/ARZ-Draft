#pragma once

#include "geometry/primitives/Point2D.h"
#include "geometry/primitives/BoundingBox2D.h"

namespace arz::rendering {

class Viewport2D final {
public:
    explicit Viewport2D(double pixelsPerMillimeter = 0.1);

    [[nodiscard]] arz::geometry::Point2D worldToScreen(
        arz::geometry::Point2D worldPoint
    ) const noexcept;
    [[nodiscard]] arz::geometry::Point2D screenToWorld(
        arz::geometry::Point2D screenPoint
    ) const noexcept;

    void setScreenOrigin(arz::geometry::Point2D origin) noexcept;
    [[nodiscard]] arz::geometry::Point2D screenOrigin() const noexcept;
    void pan(arz::geometry::Point2D screenDelta) noexcept;
    void zoomAt(
        arz::geometry::Point2D screenPoint,
        double factor
    ) noexcept;
    void zoomExtents(arz::geometry::BoundingBox2D bounds,
                     arz::geometry::Point2D screenSize,
                     double marginPixels = 40.0) noexcept;

    [[nodiscard]] double scale() const noexcept;
    [[nodiscard]] arz::geometry::BoundingBox2D visibleWorldBounds(
        arz::geometry::Point2D screenSize
    ) const noexcept;

private:
    double scale_{0.1};
    arz::geometry::Point2D screenOrigin_{};
};

}
