#include "rendering/Viewport2D.h"

#include <algorithm>
#include <cmath>

namespace arz::rendering {

Viewport2D::Viewport2D(double pixelsPerMillimeter)
    : scale_(std::clamp(pixelsPerMillimeter, 1e-6, 1e3)) {
}

arz::geometry::Point2D Viewport2D::worldToScreen(
    arz::geometry::Point2D worldPoint
) const noexcept {
    return {
        screenOrigin_.x + worldPoint.x * scale_,
        screenOrigin_.y - worldPoint.y * scale_
    };
}

arz::geometry::Point2D Viewport2D::screenToWorld(
    arz::geometry::Point2D screenPoint
) const noexcept {
    return {
        (screenPoint.x - screenOrigin_.x) / scale_,
        (screenOrigin_.y - screenPoint.y) / scale_
    };
}

void Viewport2D::setScreenOrigin(
    arz::geometry::Point2D origin
) noexcept {
    screenOrigin_ = origin;
}

arz::geometry::Point2D
Viewport2D::screenOrigin() const noexcept {
    return screenOrigin_;
}

void Viewport2D::pan(
    arz::geometry::Point2D screenDelta
) noexcept {
    screenOrigin_.x += screenDelta.x;
    screenOrigin_.y += screenDelta.y;
}

void Viewport2D::zoomAt(
    arz::geometry::Point2D screenPoint,
    double factor
) noexcept {
    if (!std::isfinite(factor) || factor <= 0.0) {
        return;
    }

    const auto anchoredWorld = screenToWorld(screenPoint);
    scale_ = std::clamp(scale_ * factor, 1e-6, 1e3);
    screenOrigin_ = {
        screenPoint.x - anchoredWorld.x * scale_,
        screenPoint.y + anchoredWorld.y * scale_
    };
}

void Viewport2D::zoomExtents(
    arz::geometry::BoundingBox2D bounds,
    arz::geometry::Point2D screenSize,
    double marginPixels
) noexcept {
    bounds = bounds.normalized();
    const double worldWidth = std::max(bounds.maxX - bounds.minX, 1.0);
    const double worldHeight = std::max(bounds.maxY - bounds.minY, 1.0);
    const double screenWidth = std::max(screenSize.x - marginPixels * 2.0, 1.0);
    const double screenHeight = std::max(screenSize.y - marginPixels * 2.0, 1.0);
    scale_ = std::clamp(std::min(screenWidth / worldWidth, screenHeight / worldHeight), 1e-6, 1e3);
    const arz::geometry::Point2D center{
        (bounds.minX + bounds.maxX) * 0.5,
        (bounds.minY + bounds.maxY) * 0.5
    };
    screenOrigin_ = {
        screenSize.x * 0.5 - center.x * scale_,
        screenSize.y * 0.5 + center.y * scale_
    };
}

double Viewport2D::scale() const noexcept {
    return scale_;
}

arz::geometry::BoundingBox2D Viewport2D::visibleWorldBounds(
    arz::geometry::Point2D screenSize
) const noexcept {
    const auto first = screenToWorld({0.0, 0.0});
    const auto second = screenToWorld({
        std::max(screenSize.x, 0.0),
        std::max(screenSize.y, 0.0)
    });
    return arz::geometry::BoundingBox2D{
        first.x, first.y, second.x, second.y
    }.normalized();
}

}
