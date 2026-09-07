#include "views/CadCanvasWidget.h"

#include <cmath>
#include <utility>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWheelEvent>

#include "app/application/CadApplicationController.h"
#include "cad/entities/CadEntity.h"

namespace arz::views {

CadCanvasWidget::CadCanvasWidget(
    arz::app::CadApplicationController& controller,
    QWidget* parent
)
    : QWidget(parent),
      controller_(controller) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::CrossCursor);
    setMinimumSize(480, 320);
}

void CadCanvasWidget::setCoordinatesChangedCallback(
    std::function<void(arz::geometry::Point2D)> callback
) {
    coordinatesChanged_ = std::move(callback);
}

void CadCanvasWidget::setStateChangedCallback(
    std::function<void()> callback
) {
    stateChanged_ = std::move(callback);
}

void CadCanvasWidget::cancelActiveTool() {
    controller_.cancel();
    snap_.reset();
    update();

    if (stateChanged_) {
        stateChanged_();
    }
}

arz::geometry::Point2D CadCanvasWidget::hoverWorldPoint() const noexcept {
    return hoverWorld_;
}

void CadCanvasWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    const auto previewStart = controller_.lineStartPoint();
    const std::optional<arz::geometry::Point2D> previewEnd =
        previewStart
        ? std::optional<arz::geometry::Point2D>(
            snap_ ? snap_->point : hoverWorld_
          )
        : std::nullopt;
    renderer_.render(
        painter,
        size(),
        controller_.document(),
        viewport_,
        controller_.selection().ids(),
        previewStart,
        previewEnd,
        snap_,
        cursorPosition_,
        controller_.overlayState()
    );
}

void CadCanvasWidget::resizeEvent(QResizeEvent* event) {
    if (!viewportInitialized_) {
        viewport_.setScreenOrigin({
            event->size().width() * 0.5,
            event->size().height() * 0.5
        });
        cursorPosition_ = QPointF(
            event->size().width() * 0.5,
            event->size().height() * 0.5
        );
        viewportInitialized_ = true;
    }

    QWidget::resizeEvent(event);
}

void CadCanvasWidget::mouseMoveEvent(QMouseEvent* event) {
    const QPointF position = event->position();

    if (panning_) {
        const QPointF delta = position - lastPanPosition_;
        viewport_.pan({delta.x(), delta.y()});
        lastPanPosition_ = position;
    }

    updateHover(position);
    controller_.updatePointer(hoverWorld_);
    update();
}

void CadCanvasWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        panning_ = true;
        lastPanPosition_ = event->position();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() == Qt::RightButton) {
        (void)controller_.rightClick();
        update();
        if (stateChanged_) stateChanged_();
        return;
    }
    if (event->button() != Qt::LeftButton) {
        return;
    }

    setFocus();
    updateHover(event->position());
    (void)controller_.canvasClick(hoverWorld_, worldTolerance(),
        event->modifiers().testFlag(Qt::ShiftModifier));
    updateHover(event->position());
    controller_.updatePointer(hoverWorld_);
    update();
    if (stateChanged_) stateChanged_();
}

void CadCanvasWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        panning_ = false;
        setCursor(Qt::CrossCursor);
        return;
    }
}

void CadCanvasWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
        std::optional<arz::geometry::BoundingBox2D> bounds;
        for (const auto id : controller_.document().objectIds()) {
            const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(
                controller_.document().object(id));
            if (!entity) continue;
            const auto box = entity->boundingBox().normalized();
            if (!bounds) bounds = box;
            else {
                bounds->minX = std::min(bounds->minX, box.minX);
                bounds->minY = std::min(bounds->minY, box.minY);
                bounds->maxX = std::max(bounds->maxX, box.maxX);
                bounds->maxY = std::max(bounds->maxY, box.maxY);
            }
        }
        if (bounds) viewport_.zoomExtents(*bounds,
            {static_cast<double>(width()), static_cast<double>(height())});
        updateHover(event->position());
        update();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CadCanvasWidget::wheelEvent(QWheelEvent* event) {
    const double factor = std::pow(
        1.0015,
        event->angleDelta().y()
    );
    viewport_.zoomAt(
        {event->position().x(), event->position().y()},
        factor
    );
    updateHover(event->position());
    update();
    event->accept();
}

void CadCanvasWidget::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        cancelActiveTool();
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Backspace) {
        controller_.backspaceCommandBuffer();
    } else if (event->key() == Qt::Key_Up) {
        controller_.selectPreviousSuggestion();
    } else if (event->key() == Qt::Key_Down) {
        controller_.selectNextSuggestion();
    } else if (event->key() == Qt::Key_Return
               || event->key() == Qt::Key_Enter
               || event->key() == Qt::Key_Space) {
        (void)controller_.confirmInput();
    } else if (!event->text().isEmpty() && event->text().front().isPrint()) {
        controller_.appendCommandCharacter(event->text().front().toLatin1());
    } else {
        QWidget::keyPressEvent(event);
        return;
    }
    update();
    if (stateChanged_) stateChanged_();
    event->accept();
}

double CadCanvasWidget::worldTolerance() const noexcept {
    return 9.0 / viewport_.scale();
}

arz::geometry::Point2D CadCanvasWidget::toWorld(
    QPointF point
) const {
    return viewport_.screenToWorld({point.x(), point.y()});
}

void CadCanvasWidget::updateHover(QPointF point) {
    cursorPosition_ = point;
    hoverWorld_ = toWorld(point);
    snap_ = controller_.snapCandidate(
        hoverWorld_,
        worldTolerance()
    );

    if (coordinatesChanged_) {
        coordinatesChanged_(hoverWorld_);
    }
}

}
