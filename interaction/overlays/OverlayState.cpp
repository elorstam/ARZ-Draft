#include "interaction/overlays/OverlayState.h"

#include <utility>

namespace arz::interaction {

void OverlayState::setDynamicText(std::string text) { dynamicText_ = std::move(text); }
const std::string& OverlayState::dynamicText() const noexcept { return dynamicText_; }
void OverlayState::setDynamicInput(DynamicInputState state) { dynamicInput_ = std::move(state); }
const DynamicInputState& OverlayState::dynamicInput() const noexcept { return dynamicInput_; }
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

void OverlayState::setCommandSuggestions(std::vector<std::string> suggestions) {
    commandSuggestions_ = std::move(suggestions);
    selectedSuggestionIndex_ = 0;
}
void OverlayState::selectPreviousSuggestion() noexcept {
    if (commandSuggestions_.empty()) return;
    selectedSuggestionIndex_ = selectedSuggestionIndex_ == 0
        ? commandSuggestions_.size() - 1 : selectedSuggestionIndex_ - 1;
}
void OverlayState::selectNextSuggestion() noexcept {
    if (!commandSuggestions_.empty())
        selectedSuggestionIndex_ = (selectedSuggestionIndex_ + 1) % commandSuggestions_.size();
}
const std::vector<std::string>& OverlayState::commandSuggestions() const noexcept {
    return commandSuggestions_;
}
std::size_t OverlayState::selectedSuggestionIndex() const noexcept { return selectedSuggestionIndex_; }
std::optional<std::string> OverlayState::selectedSuggestion() const {
    if (commandSuggestions_.empty()) return std::nullopt;
    return commandSuggestions_[selectedSuggestionIndex_];
}
void OverlayState::setPastePlacement(PastePlacementOverlay placement) {
    pastePlacement_ = std::move(placement);
}
void OverlayState::clearPastePlacement() noexcept { pastePlacement_.reset(); }
const std::optional<PastePlacementOverlay>& OverlayState::pastePlacement() const noexcept {
    return pastePlacement_;
}
void OverlayState::setDrawingPolyline(TransientPolyline value) { drawingPolyline_ = std::move(value); }
void OverlayState::clearDrawingPolyline() noexcept { drawingPolyline_.reset(); }
const std::optional<TransientPolyline>& OverlayState::drawingPolyline() const noexcept { return drawingPolyline_; }
void OverlayState::setDrawingCircle(TransientCircle value) { drawingCircle_ = value; }
void OverlayState::clearDrawingCircle() noexcept { drawingCircle_.reset(); }
const std::optional<TransientCircle>& OverlayState::drawingCircle() const noexcept { return drawingCircle_; }
void OverlayState::setDrawingArc(TransientArc value) { drawingArc_ = value; }
void OverlayState::clearDrawingArc() noexcept { drawingArc_.reset(); }
const std::optional<TransientArc>& OverlayState::drawingArc() const noexcept { return drawingArc_; }
void OverlayState::setArcReference(ArcReferenceOverlay value) { arcReference_ = value; }
void OverlayState::clearArcReference() noexcept { arcReference_.reset(); }
const std::optional<ArcReferenceOverlay>& OverlayState::arcReference() const noexcept { return arcReference_; }
void OverlayState::setDrawingLine(TransientLine value) { drawingLine_ = value; }
void OverlayState::clearDrawingLine() noexcept { drawingLine_.reset(); }
const std::optional<TransientLine>& OverlayState::drawingLine() const noexcept { return drawingLine_; }
void OverlayState::setResolvedPoint(ResolvedCadPoint value) { resolvedPoint_ = std::move(value); }
void OverlayState::clearResolvedPoint() noexcept { resolvedPoint_.reset(); }
const std::optional<ResolvedCadPoint>& OverlayState::resolvedPoint() const noexcept { return resolvedPoint_; }

}
