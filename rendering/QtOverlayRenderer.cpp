#include "rendering/QtOverlayRenderer.h"

#include <algorithm>

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
    }

    if (snap) {
        const QPointF marker = screenPoint(context, snap->point);
        painter.setPen(QPen(QColor(90, 235, 170), 1.5));
        painter.setBrush(Qt::NoBrush);
        if (snap->type == arz::cad::SnapType::Endpoint) {
            painter.drawRect(QRectF(marker.x() - 5.0, marker.y() - 5.0, 10.0, 10.0));
        } else {
            QPolygonF triangle;
            triangle << QPointF(marker.x(), marker.y() - 6.0)
                     << QPointF(marker.x() - 6.0, marker.y() + 5.0)
                     << QPointF(marker.x() + 6.0, marker.y() + 5.0);
            painter.drawPolygon(triangle);
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
    painter.drawLine(QPointF(0.0, cursorPosition.y()),
                     QPointF(canvasSize.width(), cursorPosition.y()));
    painter.drawLine(QPointF(cursorPosition.x(), 0.0),
                     QPointF(cursorPosition.x(), canvasSize.height()));

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
