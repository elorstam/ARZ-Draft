#include "interaction/overlays/OverlayState.h"

#include <utility>

namespace arz::interaction {

void OverlayState::setDynamicText(std::string text) { dynamicText_ = std::move(text); }
const std::string& OverlayState::dynamicText() const noexcept { return dynamicText_; }
void OverlayState::beginSelectionWindow(arz::geometry::Point2D point) {
    selectionWindow_ = SelectionWindowOverlay{point, point, false};
}
void OverlayState::updateSelectionWindow(arz::geometry::Point2D point) {
    if (selectionWindow_) {
        selectionWindow_->current = point;
        selectionWindow_->crossing = point.x < selectionWindow_->first.x;
    }
}
void OverlayState::clearSelectionWindow() noexcept { selectionWindow_.reset(); }
const std::optional<SelectionWindowOverlay>& OverlayState::selectionWindow() const noexcept {
    return selectionWindow_;
}

}
