#include "rendering/QtDocumentRenderer.h"

#include <algorithm>

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <numbers>

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
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, LineRenderPrimitive>)
                drawLine(painter, context, value);
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, PolylineRenderPrimitive>)
                drawPolyline(painter, context, value);
            else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, CircleRenderPrimitive>)
                drawCircle(painter, context, value);
            else drawArc(painter, context, value);
        }, primitive);
    }
}

namespace {
void setEntityPen(QPainter& painter, const RenderStyle& style) {
    const double width = ScreenLineWeightPolicy::pixelWidth(style.lineWeight);
    painter.setPen(QPen(style.selected ? QColor(255, 205, 64) : qtColor(style.color),
        style.selected ? std::max(2.5, width) : width, Qt::SolidLine, Qt::RoundCap,
        Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
}
}

void QtDocumentRenderer::drawPolyline(QPainter& painter, const RenderContext& context,
    const PolylineRenderPrimitive& value) const {
    setEntityPen(painter, value.style);
    QPolygonF polygon;
    for (const auto point : value.vertices) polygon << screenPoint(context, point);
    if (value.closed && !value.vertices.empty()) polygon << screenPoint(context, value.vertices.front());
    painter.drawPolyline(polygon);
}

void QtDocumentRenderer::drawCircle(QPainter& painter, const RenderContext& context,
    const CircleRenderPrimitive& value) const {
    setEntityPen(painter, value.style);
    const auto center = screenPoint(context, value.center);
    const double radius = value.radius * context.pixelsPerWorldUnit;
    painter.drawEllipse(center, radius, radius);
}

void QtDocumentRenderer::drawArc(QPainter& painter, const RenderContext& context,
    const ArcRenderPrimitive& value) const {
    setEntityPen(painter, value.style);
    const auto center = screenPoint(context, value.center);
    const double radius = value.radius * context.pixelsPerWorldUnit;
    const QRectF bounds(center.x() - radius, center.y() - radius, radius * 2.0, radius * 2.0);
    const int start = static_cast<int>(std::lround(value.startAngle * 180.0 / std::numbers::pi * 16.0));
    const double signedSweep = value.counterClockwise ? value.sweepAngle : -value.sweepAngle;
    const int span = static_cast<int>(std::lround(signedSweep * 180.0 / std::numbers::pi * 16.0));
    painter.drawArc(bounds, start, span);
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
