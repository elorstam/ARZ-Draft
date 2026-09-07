#pragma once

#include <functional>
#include <optional>

#include <QPointF>
#include <QWidget>

#include "cad/snapping/SnapResult.h"
#include "geometry/primitives/Point2D.h"
#include "rendering/QtDocumentRenderer.h"
#include "rendering/QtOverlayRenderer.h"
#include "rendering/Viewport2D.h"

namespace arz::app {
class CadApplicationController;
}

namespace arz::views {

class CadCanvasWidget final : public QWidget {
public:
    explicit CadCanvasWidget(
        arz::app::CadApplicationController& controller,
        QWidget* parent = nullptr
    );

    void setCoordinatesChangedCallback(
        std::function<void(arz::geometry::Point2D)> callback
    );
    void setStateChangedCallback(std::function<void()> callback);
    void cancelActiveTool();
    void refreshInteractionPointer();
    [[nodiscard]] arz::geometry::Point2D hoverWorldPoint() const noexcept;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    [[nodiscard]] double worldTolerance() const noexcept;
    [[nodiscard]] arz::geometry::Point2D toWorld(QPointF point) const;
    void updateHover(QPointF point);

    arz::app::CadApplicationController& controller_;
    arz::rendering::Viewport2D viewport_;
    arz::rendering::QtDocumentRenderer documentRenderer_;
    arz::rendering::QtOverlayRenderer overlayRenderer_;
    QPointF cursorPosition_{};
    QPointF lastPanPosition_{};
    arz::geometry::Point2D hoverWorld_{};
    std::optional<arz::cad::SnapResult> snap_;
    std::function<void(arz::geometry::Point2D)> coordinatesChanged_;
    std::function<void()> stateChanged_;
    bool panning_{false};
    bool viewportInitialized_{false};
};

}
