#pragma once

#include <optional>
#include <vector>

#include <QPointF>
#include <QSize>

#include "cad/snapping/SnapResult.h"
#include "core/objects/ObjectId.h"
#include "geometry/primitives/Point2D.h"
#include "interaction/overlays/OverlayState.h"

class QPainter;

namespace arz::core {
class Document;
}

namespace arz::rendering {

class Viewport2D;

class QtDocumentRenderer final {
public:
    void render(
        QPainter& painter,
        const QSize& canvasSize,
        const arz::core::Document& document,
        const Viewport2D& viewport,
        const std::vector<arz::core::ObjectId>& selectedObjectIds,
        std::optional<arz::geometry::Point2D> previewStart,
        std::optional<arz::geometry::Point2D> previewEnd,
        const std::optional<arz::cad::SnapResult>& snap,
        QPointF cursorPosition,
        const arz::interaction::OverlayState& overlays
    ) const;
};

}
