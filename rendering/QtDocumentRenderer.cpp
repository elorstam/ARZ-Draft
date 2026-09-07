#include "rendering/QtDocumentRenderer.h"

#include <algorithm>

#include <QColor>
#include <QPainter>
#include <QPen>

#include "core/document/Document.h"
#include "rendering/ScreenLineWeightPolicy.h"

namespace arz::rendering {

namespace {

QPointF screenPoint(const RenderContext& context,
                    arz::geometry::Point2D point) {
    const auto screen = context.viewport.worldToScreen(point);
    return {screen.x, screen.y};
}

QColor qtColor(arz::cad::Color color) {
    return {color.red, color.green, color.blue};
}

}

void QtDocumentRenderer::render(
    QPainter& painter,
    const QSize& canvasSize,
    const arz::core::Document& document,
    const RenderContext& context
) {
    painter.fillRect(QRect(QPoint(0, 0), canvasSize), QColor(24, 29, 36));
    painter.setRenderHint(QPainter::Antialiasing, true);

    const auto origin = context.viewport.worldToScreen({0.0, 0.0});
    painter.setPen(QPen(QColor(48, 57, 68), 1.0));
    painter.drawLine(QPointF(0.0, origin.y),
                     QPointF(canvasSize.width(), origin.y));
    painter.drawLine(QPointF(origin.x, 0.0),
                     QPointF(origin.x, canvasSize.height()));

    planner_.build(document, context, scene_);
    for (const auto& primitive : scene_.primitives()) {
        std::visit([this, &painter, &context](const auto& value) {
            drawLine(painter, context, value);
        }, primitive);
    }
}

void QtDocumentRenderer::drawLine(
    QPainter& painter,
    const RenderContext& context,
    const LineRenderPrimitive& line
) const {
    const double modelWidth = ScreenLineWeightPolicy::pixelWidth(
        line.style.lineWeight);
    painter.setPen(QPen(
        line.style.selected ? QColor(255, 205, 64) : qtColor(line.style.color),
        line.style.selected ? std::max(2.5, modelWidth) : modelWidth,
        Qt::SolidLine,
        Qt::RoundCap
    ));
    painter.drawLine(screenPoint(context, line.start),
                     screenPoint(context, line.end));
}

std::optional<arz::geometry::BoundingBox2D>
QtDocumentRenderer::visibleDocumentBounds(
    const arz::core::Document& document
) const {
    return planner_.visibleDocumentBounds(document);
}

}
