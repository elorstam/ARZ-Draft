#pragma once

#include <optional>

#include <QPointF>
#include <QSize>

#include "interaction/overlays/OverlayState.h"
#include "rendering/RenderContext.h"

class QPainter;

namespace arz::rendering {

class QtOverlayRenderer final {
public:
    static constexpr double CrosshairArmLengthPixels = 36.0;
    static constexpr double PickboxSizePixels = 8.0;

    void render(QPainter& painter,
                const QSize& canvasSize,
                const RenderContext& context,
                QPointF cursorPosition,
                const arz::interaction::OverlayState& overlays) const;
};

}
