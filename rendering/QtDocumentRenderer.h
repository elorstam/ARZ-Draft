#pragma once

#include <optional>

#include <QSize>

#include "rendering/DocumentRenderPlanner.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"

class QPainter;

namespace arz::core { class Document; }

namespace arz::rendering {

class QtDocumentRenderer final {
public:
    void render(QPainter& painter,
                const QSize& canvasSize,
                const arz::core::Document& document,
                const RenderContext& context);

    [[nodiscard]] std::optional<arz::geometry::BoundingBox2D>
    visibleDocumentBounds(const arz::core::Document& document) const;

private:
    void drawLine(QPainter& painter,
                  const RenderContext& context,
                  const LineRenderPrimitive& line) const;

    DocumentRenderPlanner planner_;
    RenderScene scene_;
};

}
