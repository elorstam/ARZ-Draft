#pragma once

#include <algorithm>
#include <span>

#include "core/objects/ObjectId.h"
#include "geometry/primitives/BoundingBox2D.h"
#include "geometry/primitives/Point2D.h"
#include "rendering/Viewport2D.h"

namespace arz::rendering {

struct RenderContext final {
    const Viewport2D& viewport;
    arz::geometry::BoundingBox2D visibleWorldBounds;
    arz::geometry::Point2D screenSize;
    std::span<const arz::core::ObjectId> selectedObjectIds;
    double pixelsPerWorldUnit{1.0};

    [[nodiscard]] bool isSelected(arz::core::ObjectId id) const noexcept {
        return std::ranges::find(selectedObjectIds, id) != selectedObjectIds.end();
    }
};

[[nodiscard]] inline RenderContext makeRenderContext(
    const Viewport2D& viewport,
    arz::geometry::Point2D screenSize,
    std::span<const arz::core::ObjectId> selectedObjectIds
) noexcept {
    return {
        viewport,
        viewport.visibleWorldBounds(screenSize),
        screenSize,
        selectedObjectIds,
        viewport.scale()
    };
}

}
