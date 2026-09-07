#pragma once

#include <optional>

#include <QPointF>
#include <QSize>

#include "cad/snapping/SnapResult.h"
#include "geometry/primitives/Point2D.h"
#include "interaction/overlays/OverlayState.h"
#include "rendering/RenderContext.h"

class QPainter;

namespace arz::rendering {

class QtOverlayRenderer final {
public:
    void render(QPainter& painter,
                const QSize& canvasSize,
                const RenderContext& context,
                std::optional<arz::geometry::Point2D> linePreviewStart,
                std::optional<arz::geometry::Point2D> linePreviewEnd,
                const std::optional<arz::cad::SnapResult>& snap,
                QPointF cursorPosition,
                const arz::interaction::OverlayState& overlays) const;
};

}
