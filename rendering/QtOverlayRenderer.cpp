#include "rendering/QtOverlayRenderer.h"

#include <algorithm>
#include <cmath>
#include <numbers>

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPolygonF>

namespace arz::rendering {

namespace {

QPointF screenPoint(const RenderContext& context,
                    arz::geometry::Point2D point) {
    const auto screen = context.viewport.worldToScreen(point);
    return {screen.x, screen.y};
}

}

void QtOverlayRenderer::render(
    QPainter& painter,
    const QSize& canvasSize,
    const RenderContext& context,
    std::optional<arz::geometry::Point2D> linePreviewStart,
    std::optional<arz::geometry::Point2D> linePreviewEnd,
    const std::optional<arz::cad::SnapResult>& snap,
    QPointF cursorPosition,
    const arz::interaction::OverlayState& overlays
) const {
    if (linePreviewStart && linePreviewEnd) {
        painter.setPen(QPen(QColor(120, 210, 255), 1.2, Qt::DashLine));
        painter.drawLine(screenPoint(context, *linePreviewStart),
                         screenPoint(context, *linePreviewEnd));
    }

    if (const auto& paste = overlays.pastePlacement()) {
        painter.setPen(QPen(QColor(110, 220, 255, 210), 1.4, Qt::DashLine));
        for (const auto& line : paste->lines) {
            painter.drawLine(screenPoint(context, line.start),
                             screenPoint(context, line.end));
        }
        for (const auto& polyline : paste->polylines) {
            QPolygonF polygon;
            for (const auto point : polyline.vertices) polygon << screenPoint(context, point);
            if (polyline.closed && !polyline.vertices.empty())
                polygon << screenPoint(context, polyline.vertices.front());
            painter.drawPolyline(polygon);
        }
        for (const auto& circle : paste->circles) {
            const auto center = screenPoint(context, circle.center);
            const double radius = circle.radius * context.pixelsPerWorldUnit;
            painter.drawEllipse(center, radius, radius);
        }
        for (const auto& arc : paste->arcs) {
            const auto center = screenPoint(context, arc.center);
            const double radius = arc.radius * context.pixelsPerWorldUnit;
            const QRectF bounds(center.x() - radius, center.y() - radius,
                                radius * 2.0, radius * 2.0);
            const int start = static_cast<int>(std::lround(
                arc.startAngle * 180.0 / std::numbers::pi * 16.0));
            const int span = static_cast<int>(std::lround(
                (arc.counterClockwise ? arc.sweepAngle : -arc.sweepAngle)
                * 180.0 / std::numbers::pi * 16.0));
            painter.drawArc(bounds, start, span);
        }
    }

    painter.setPen(QPen(QColor(120, 210, 255), 1.2, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    if (const auto& polyline = overlays.drawingPolyline()) {
        QPolygonF polygon;
        for (const auto point : polyline->vertices) polygon << screenPoint(context, point);
        if (polyline->closed && !polyline->vertices.empty())
            polygon << screenPoint(context, polyline->vertices.front());
        painter.drawPolyline(polygon);
    }
    if (const auto& circle = overlays.drawingCircle()) {
        const auto center = screenPoint(context, circle->center);
        const double radius = circle->radius * context.pixelsPerWorldUnit;
        painter.drawEllipse(center, radius, radius);
    }
    if (const auto& reference = overlays.arcReference()) {
        painter.setPen(QPen(QColor(150, 205, 230, 190), 1.0, Qt::DashLine));
        painter.drawLine(screenPoint(context, reference->startPoint),
                         screenPoint(context, reference->secondPoint));
        painter.drawLine(screenPoint(context, reference->secondPoint),
                         screenPoint(context, reference->provisionalEndPoint));
        painter.setPen(QPen(QColor(150, 205, 230, 135), 1.0, Qt::DotLine));
        painter.drawLine(screenPoint(context, reference->startPoint),
                         screenPoint(context, reference->provisionalEndPoint));
    }
    if (const auto& arc = overlays.drawingArc()) {
        const auto center = screenPoint(context, arc->center);
        const double radius = arc->radius * context.pixelsPerWorldUnit;
        const QRectF bounds(center.x() - radius, center.y() - radius,
                            radius * 2.0, radius * 2.0);
        const int start = static_cast<int>(std::lround(
            arc->startAngle * 180.0 / std::numbers::pi * 16.0));
        const int span = static_cast<int>(std::lround(
            (arc->counterClockwise ? arc->sweepAngle : -arc->sweepAngle)
            * 180.0 / std::numbers::pi * 16.0));
        painter.drawArc(bounds, start, span);
        painter.setPen(QPen(QColor(150, 205, 230, 180), 1.0, Qt::DotLine));
        painter.drawLine(screenPoint(context, arc->center),
                         screenPoint(context, arc->throughPoint));
        painter.drawLine(screenPoint(context, arc->center),
                         screenPoint(context, arc->endPoint));
        painter.drawLine(screenPoint(context, arc->throughPoint),
                         screenPoint(context, arc->endPoint));
    }

    if (snap) {
        const QPointF marker = screenPoint(context, snap->point);
        painter.setPen(QPen(QColor(90, 235, 170), 1.5));
        painter.setBrush(Qt::NoBrush);
        if (snap->type == arz::cad::SnapType::Endpoint) {
            painter.drawRect(QRectF(marker.x() - 5.0, marker.y() - 5.0, 10.0, 10.0));
        } else if (snap->type == arz::cad::SnapType::Midpoint) {
            QPolygonF triangle;
            triangle << QPointF(marker.x(), marker.y() - 6.0)
                     << QPointF(marker.x() - 6.0, marker.y() + 5.0)
                     << QPointF(marker.x() + 6.0, marker.y() + 5.0);
            painter.drawPolygon(triangle);
        } else if (snap->type == arz::cad::SnapType::Center) {
            painter.drawEllipse(marker, 5.0, 5.0);
        } else {
            QPolygonF diamond;
            diamond << QPointF(marker.x(), marker.y() - 6.0)
                    << QPointF(marker.x() + 6.0, marker.y())
                    << QPointF(marker.x(), marker.y() + 6.0)
                    << QPointF(marker.x() - 6.0, marker.y());
            painter.drawPolygon(diamond);
        }
    }

    if (const auto& window = overlays.selectionWindow()) {
        const QPointF first = screenPoint(context, window->first);
        const QPointF current = screenPoint(context, window->current);
        const QColor color = window->crossing
            ? QColor(70, 170, 110, 70) : QColor(60, 130, 220, 70);
        painter.setPen(QPen(color.lighter(180), 1.0,
            window->crossing ? Qt::DashLine : Qt::SolidLine));
        painter.setBrush(color);
        painter.drawRect(QRectF(first, current).normalized());
    }

    painter.setPen(QPen(QColor(178, 190, 204, 170), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(cursorPosition.x() - CrosshairArmLengthPixels, cursorPosition.y()),
                     QPointF(cursorPosition.x() + CrosshairArmLengthPixels, cursorPosition.y()));
    painter.drawLine(QPointF(cursorPosition.x(), cursorPosition.y() - CrosshairArmLengthPixels),
                     QPointF(cursorPosition.x(), cursorPosition.y() + CrosshairArmLengthPixels));
    painter.setPen(QPen(QColor(235, 240, 245, 220), 1.0));
    const double pickboxHalf = PickboxSizePixels * 0.5;
    painter.drawRect(QRectF(cursorPosition.x() - pickboxHalf,
                            cursorPosition.y() - pickboxHalf,
                            PickboxSizePixels, PickboxSizePixels));

    if (overlays.dynamicText().empty()) return;
    const QString text = QString::fromStdString(overlays.dynamicText());
    const QRect textBounds = painter.fontMetrics().boundingRect(text);
    const QRectF box(cursorPosition + QPointF(18.0, 18.0),
        QSizeF(textBounds.width() + 12.0, textBounds.height() + 8.0));
    painter.fillRect(box, QColor(35, 43, 52, 235));
    painter.setPen(QColor(235, 240, 245));
    painter.drawText(box, Qt::AlignCenter, text);
    double suggestionY = box.bottom() + 2.0;
    for (std::size_t index = 0;
         index < overlays.commandSuggestions().size(); ++index) {
        const QString suggestion = QString::fromStdString(
            overlays.commandSuggestions()[index]);
        const QRect suggestionBounds = painter.fontMetrics().boundingRect(suggestion);
        const QRectF suggestionBox(QPointF(box.left(), suggestionY),
            QSizeF(std::max(box.width(), suggestionBounds.width() + 18.0),
                   suggestionBounds.height() + 8.0));
        painter.fillRect(suggestionBox,
            index == overlays.selectedSuggestionIndex()
                ? QColor(32, 115, 160, 245) : QColor(30, 37, 45, 245));
        painter.setPen(QColor(235, 240, 245));
        painter.drawText(suggestionBox.adjusted(7.0, 0.0, -4.0, 0.0),
                         Qt::AlignVCenter | Qt::AlignLeft, suggestion);
        suggestionY = suggestionBox.bottom() + 1.0;
    }
}

}
