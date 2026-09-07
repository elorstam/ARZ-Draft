#include "app/application/CadApplicationController.h"

#include <array>
#include <memory>
#include <utility>
#include <vector>

#include "cad/commands/AddLineCommand.h"
#include "cad/commands/AddLinesCommand.h"
#include "cad/commands/DeleteEntitiesCommand.h"
#include "cad/entities/CadEntity.h"
#include "cad/spatial/SpatialIndexSynchronizer.h"

namespace arz::app {

namespace {
constexpr std::array EnabledSnapTypes{arz::cad::SnapType::Endpoint,
                                      arz::cad::SnapType::Midpoint};
}

CadApplicationController::CadApplicationController()
    : selectionService_(document_, spatialIndex_, pickRefiner_),
      snapService_(document_, spatialIndex_, snapProvider_) {
    updateOverlayText();
}

void CadApplicationController::startLine() noexcept {
    if (const auto descriptor = commandRegistry_.resolve("LINE")) (void)invoke(*descriptor);
}
void CadApplicationController::cancel() noexcept { (void)escape(); }

void CadApplicationController::appendCommandCharacter(char character) {
    if (lineInput_.state() == arz::interaction::LineInputState::Inactive) {
        commandInput_.append(character);
        updateOverlayText();
    }
}
void CadApplicationController::backspaceCommandBuffer() noexcept {
    if (lineInput_.state() == arz::interaction::LineInputState::Inactive) {
        commandInput_.backspace();
        updateOverlayText();
    }
}
void CadApplicationController::setCommandBuffer(std::string text) {
    if (lineInput_.state() == arz::interaction::LineInputState::Inactive) {
        commandInput_.setBuffer(std::move(text));
        updateOverlayText();
    }
}

bool CadApplicationController::confirmInput() {
    if (!commandInput_.buffer().empty()) {
        const auto text = overlayState_.selectedSuggestion().value_or(commandInput_.buffer());
        commandInput_.clearBuffer();
        return invokeCommandText(text);
    }
    if (overlayState_.pastePlacement()) {
        return commitPaste(overlayState_.pastePlacement()->insertionPoint);
    }
    if (lineInput_.state() != arz::interaction::LineInputState::Inactive) {
        lineInput_.cancel();
        updateOverlayText();
        return true;
    }
    return commandInput_.lastRepeatable()
        && invokeCommandText(*commandInput_.lastRepeatable());
}

bool CadApplicationController::escape() {
    if (overlayState_.selectionWindow()) {
        overlayState_.clearSelectionWindow();
    } else if (overlayState_.pastePlacement()) {
        overlayState_.clearPastePlacement();
    } else if (lineInput_.state() != arz::interaction::LineInputState::Inactive) {
        lineInput_.cancel();
    } else if (!commandInput_.buffer().empty()) {
        commandInput_.clearBuffer();
    } else if (!selection_.empty()) {
        selection_.clear();
    } else {
        return false;
    }
    updateOverlayText();
    return true;
}

bool CadApplicationController::invokeCommandText(std::string_view text) {
    const auto descriptor = commandRegistry_.resolve(text);
    if (!descriptor) {
        overlayState_.setDynamicText("Unknown command");
        return false;
    }
    return invoke(*descriptor);
}
bool CadApplicationController::rightClick() { return confirmInput(); }
void CadApplicationController::selectPreviousSuggestion() noexcept {
    overlayState_.selectPreviousSuggestion();
}
void CadApplicationController::selectNextSuggestion() noexcept {
    overlayState_.selectNextSuggestion();
}

void CadApplicationController::updatePointer(arz::geometry::Point2D worldPoint,
                                             double worldTolerance) {
    if (overlayState_.selectionWindow()) overlayState_.updateSelectionWindow(worldPoint);
    if (!overlayState_.pastePlacement()) return;
    if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
    const auto base = clipboard_.basePoint();
    const arz::geometry::Point2D delta{worldPoint.x - base.x, worldPoint.y - base.y};
    arz::interaction::PastePlacementOverlay placement{base, worldPoint, {}};
    for (const auto& source : clipboard_.lines()) {
        placement.lines.push_back({
            {source.start.x + delta.x, source.start.y + delta.y},
            {source.end.x + delta.x, source.end.y + delta.y}
        });
    }
    overlayState_.setPastePlacement(std::move(placement));
}

bool CadApplicationController::invoke(const arz::interaction::CommandDescriptor& descriptor) {
    bool succeeded = false;
    switch (descriptor.command) {
    case arz::interaction::CadCommand::Line:
        overlayState_.clearPastePlacement();
        lineInput_.activate();
        succeeded = true;
        break;
    case arz::interaction::CadCommand::Undo: succeeded = undo(); break;
    case arz::interaction::CadCommand::Redo: succeeded = redo(); break;
    case arz::interaction::CadCommand::Cancel: succeeded = escape(); break;
    case arz::interaction::CadCommand::Unknown: break;
    }
    if (succeeded) commandInput_.recordInvocation(descriptor.canonicalName, descriptor.repeatable);
    updateOverlayText();
    return succeeded;
}

CanvasAction CadApplicationController::canvasClick(arz::geometry::Point2D worldPoint,
                                                    double worldTolerance,
                                                    bool shiftModifier) {
    if (overlayState_.pastePlacement()) {
        if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
        return commitPaste(worldPoint) ? CanvasAction::EntityCreated : CanvasAction::None;
    }
    if (lineInput_.state() != arz::interaction::LineInputState::Inactive) {
        if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
        const auto input = lineInput_.acceptPoint(worldPoint);
        updateOverlayText();
        if (!input) return CanvasAction::FirstLinePointAccepted;
        auto command = std::make_unique<arz::cad::AddLineCommand>(
            document_, currentLayerId_, input->start, input->end);
        auto* view = command.get();
        if (!history_.execute(std::move(command))) return CanvasAction::None;
        selection_.replace({view->objectId()});
        if (!rebuildSpatialIndex()) {
            (void)history_.undo();
            (void)rebuildSpatialIndex();
            selection_.clear();
            return CanvasAction::None;
        }
        return CanvasAction::EntityCreated;
    }

    if (overlayState_.selectionWindow()) {
        overlayState_.updateSelectionWindow(worldPoint);
        return finishSelectionWindow(shiftModifier)
            ? CanvasAction::SelectionChanged : CanvasAction::None;
    }
    const auto picked = selectionService_.pointPick(worldPoint, worldTolerance);
    if (picked.empty()) {
        if (!shiftModifier) selection_.clear();
        overlayState_.beginSelectionWindow(worldPoint);
        updateOverlayText();
        return CanvasAction::SelectionWindowStarted;
    } else if (shiftModifier) {
        selection_.toggle(picked.front());
    } else {
        selection_.add(picked.front());
    }
    return CanvasAction::SelectionChanged;
}

void CadApplicationController::beginSelectionWindow(arz::geometry::Point2D first) {
    if (lineInput_.state() == arz::interaction::LineInputState::Inactive)
        overlayState_.beginSelectionWindow(first);
}
void CadApplicationController::updateSelectionWindow(arz::geometry::Point2D current) {
    overlayState_.updateSelectionWindow(current);
}
bool CadApplicationController::finishSelectionWindow(bool removalMode) {
    const auto window = overlayState_.selectionWindow();
    if (!window) return false;
    const auto ids = window->crossing
        ? selectionService_.crossingWindow(window->first, window->current)
        : selectionService_.containedWindow(window->first, window->current);
    if (removalMode) selection_.removeAll(ids); else selection_.addAll(ids);
    overlayState_.clearSelectionWindow();
    updateOverlayText();
    return true;
}

std::optional<arz::cad::SnapResult> CadApplicationController::snapCandidate(
    arz::geometry::Point2D worldPoint, double worldTolerance) const {
    if ((lineInput_.state() == arz::interaction::LineInputState::Inactive
         && !overlayState_.pastePlacement())
        || !draftingSettings_.enabled(arz::interaction::DraftingToggle::ObjectSnap))
        return std::nullopt;
    return snapService_.bestSnap(worldPoint, worldTolerance, EnabledSnapTypes);
}

bool CadApplicationController::undo() {
    lineInput_.cancel();
    overlayState_.clearPastePlacement();
    overlayState_.clearSelectionWindow();
    if (!history_.undo()) return false;
    if (!rebuildSpatialIndex()) {
        (void)history_.redo();
        (void)rebuildSpatialIndex();
        return false;
    }
    synchronizeAfterModelChange();
    return true;
}
bool CadApplicationController::redo() {
    lineInput_.cancel();
    overlayState_.clearPastePlacement();
    overlayState_.clearSelectionWindow();
    if (!history_.redo()) return false;
    if (!rebuildSpatialIndex()) {
        (void)history_.undo();
        (void)rebuildSpatialIndex();
        return false;
    }
    synchronizeAfterModelChange();
    return true;
}
bool CadApplicationController::deleteSelection() {
    if (selection_.empty()) return false;
    if (!history_.execute(std::make_unique<arz::cad::DeleteEntitiesCommand>(document_, selection_.ids())))
        return false;
    selection_.clear();
    synchronizeAfterModelChange();
    return true;
}
bool CadApplicationController::copySelection() {
    return clipboard_.copy(document_, selection_.ids());
}
bool CadApplicationController::cutSelection() { return copySelection() && deleteSelection(); }
bool CadApplicationController::paste() {
    if (clipboard_.empty()) return false;
    lineInput_.cancel();
    commandInput_.clearBuffer();
    overlayState_.clearSelectionWindow();
    const auto base = clipboard_.basePoint();
    arz::interaction::PastePlacementOverlay placement{base, base, {}};
    for (const auto& source : clipboard_.lines())
        placement.lines.push_back({source.start, source.end});
    overlayState_.setPastePlacement(std::move(placement));
    updateOverlayText();
    return true;
}

bool CadApplicationController::commitPaste(arz::geometry::Point2D insertionPoint) {
    if (clipboard_.empty() || !overlayState_.pastePlacement()) return false;
    const auto base = clipboard_.basePoint();
    const arz::geometry::Point2D delta{insertionPoint.x - base.x, insertionPoint.y - base.y};
    std::vector<arz::cad::LineCreationData> lines;
    for (const auto& source : clipboard_.lines()) {
        lines.push_back({source.layerId,
            {source.start.x + delta.x, source.start.y + delta.y},
            {source.end.x + delta.x, source.end.y + delta.y}, source.graphics});
    }
    auto command = std::make_unique<arz::cad::AddLinesCommand>(document_, std::move(lines));
    auto* view = command.get();
    if (!history_.execute(std::move(command))) return false;
    selection_.replace(view->objectIds());
    overlayState_.clearPastePlacement();
    synchronizeAfterModelChange();
    return true;
}
bool CadApplicationController::pastePlacementActive() const noexcept {
    return overlayState_.pastePlacement().has_value();
}
bool CadApplicationController::selectAll() {
    std::vector<arz::core::ObjectId> selectable;
    for (const auto id : document_.objectIds()) {
        const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(document_.object(id));
        if (!entity) continue;
        const auto* layer = document_.layers().get(entity->layerId());
        if (layer && layer->visible() && !layer->frozen() && !layer->locked()) selectable.push_back(id);
    }
    selection_.replace(std::move(selectable));
    return !selection_.empty();
}

const arz::core::Document& CadApplicationController::document() const noexcept { return document_; }
const arz::cad::ISpatialIndex2D& CadApplicationController::spatialIndex() const noexcept { return spatialIndex_; }
const arz::core::TransactionHistory& CadApplicationController::history() const noexcept { return history_; }
const arz::interaction::SelectionSet& CadApplicationController::selection() const noexcept { return selection_; }
arz::core::ObjectId CadApplicationController::selectedObjectId() const noexcept {
    return selection_.size() == 1 ? selection_.ids().front() : arz::core::InvalidObjectId;
}
const arz::interaction::CadClipboard& CadApplicationController::clipboard() const noexcept { return clipboard_; }
const arz::interaction::DraftingSettings& CadApplicationController::draftingSettings() const noexcept { return draftingSettings_; }
const arz::interaction::OverlayState& CadApplicationController::overlayState() const noexcept { return overlayState_; }
const arz::interaction::CommandInputState& CadApplicationController::commandInput() const noexcept { return commandInput_; }
bool CadApplicationController::toggleDrafting(arz::interaction::DraftingToggle toggle) noexcept {
    const bool value = draftingSettings_.toggle(toggle);
    updateOverlayText();
    return value;
}
arz::cad::LayerId CadApplicationController::currentLayerId() const noexcept { return currentLayerId_; }
bool CadApplicationController::setCurrentLayerId(arz::cad::LayerId layerId) noexcept {
    if (!document_.layers().contains(layerId)) return false;
    currentLayerId_ = layerId;
    return true;
}
arz::interaction::LineInputState CadApplicationController::lineInputState() const noexcept { return lineInput_.state(); }
std::optional<arz::geometry::Point2D> CadApplicationController::lineStartPoint() const noexcept { return lineInput_.firstPoint(); }
std::string CadApplicationController::commandPrompt() const {
    using arz::interaction::LineInputState;
    if (overlayState_.pastePlacement()) return "PASTE: Specify insertion point";
    if (overlayState_.selectionWindow()) return "Specify opposite corner";
    if (lineInput_.state() == LineInputState::AwaitingFirstPoint) return "LINE: Specify first point";
    if (lineInput_.state() == LineInputState::AwaitingSecondPoint) return "LINE: Specify next point";
    return "Command:";
}
bool CadApplicationController::rebuildSpatialIndex() {
    return arz::cad::SpatialIndexSynchronizer::rebuild(document_, spatialIndex_);
}
void CadApplicationController::synchronizeAfterModelChange() {
    (void)rebuildSpatialIndex();
    selection_.removeStale(document_);
    overlayState_.clearSelectionWindow();
    updateOverlayText();
}
void CadApplicationController::updateOverlayText() {
    using arz::interaction::DraftingToggle;
    using arz::interaction::LineInputState;
    std::vector<std::string> suggestions;
    for (const auto& descriptor : commandRegistry_.suggest(commandInput_.buffer()))
        suggestions.push_back(descriptor.canonicalName);
    overlayState_.setCommandSuggestions(std::move(suggestions));
    if (!draftingSettings_.enabled(DraftingToggle::DynamicInput)) overlayState_.setDynamicText({});
    else if (!commandInput_.buffer().empty()) overlayState_.setDynamicText(commandInput_.buffer());
    else if (overlayState_.pastePlacement()) overlayState_.setDynamicText("Specify insertion point");
    else if (overlayState_.selectionWindow()) overlayState_.setDynamicText("Specify opposite corner");
    else if (lineInput_.state() == LineInputState::AwaitingFirstPoint) overlayState_.setDynamicText("Specify first point");
    else if (lineInput_.state() == LineInputState::AwaitingSecondPoint) overlayState_.setDynamicText("Specify next point");
    else overlayState_.setDynamicText({});
}

}
