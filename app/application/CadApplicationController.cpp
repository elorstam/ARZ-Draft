#include "app/application/CadApplicationController.h"

#include <array>
#include <cctype>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

#include "cad/commands/AddLineCommand.h"
#include "cad/commands/AddPolylineCommand.h"
#include "cad/commands/AddCircleCommand.h"
#include "cad/commands/AddArcCommand.h"
#include "cad/commands/AddCadEntitiesCommand.h"
#include "cad/commands/DeleteEntitiesCommand.h"
#include "cad/entities/CadEntity.h"
#include "cad/spatial/SpatialIndexSynchronizer.h"
#include "geometry/algorithms/Curve2D.h"
#include "geometry/algorithms/Point2DOperations.h"

namespace arz::app {

namespace {
constexpr std::array EnabledSnapTypes{arz::cad::SnapType::Endpoint,
                                      arz::cad::SnapType::Midpoint,
                                      arz::cad::SnapType::Center,
                                      arz::cad::SnapType::Quadrant};
}

CadApplicationController::CadApplicationController()
    : selectionService_(document_, spatialIndex_, pickRefiner_),
      snapService_(document_, spatialIndex_, snapProvider_) {
    updateOverlayText();
}

void CadApplicationController::startLine() noexcept {
    if (const auto descriptor = commandRegistry_.resolve("LINE")) (void)invoke(*descriptor);
}
void CadApplicationController::startPolyline() noexcept {
    if (const auto descriptor = commandRegistry_.resolve("PLINE")) (void)invoke(*descriptor);
}
void CadApplicationController::startCircle() noexcept {
    if (const auto descriptor = commandRegistry_.resolve("CIRCLE")) (void)invoke(*descriptor);
}
void CadApplicationController::startArc() noexcept {
    if (const auto descriptor = commandRegistry_.resolve("ARC")) (void)invoke(*descriptor);
}
void CadApplicationController::cancel() noexcept { (void)escape(); }

void CadApplicationController::appendCommandCharacter(char character) {
    if (polylineInput_.state() != arz::interaction::PolylineInputState::Inactive) {
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        if (upper >= 'A' && upper <= 'Z') activeOptionBuffer_.push_back(upper);
        updateOverlayText();
    } else if (!anyDrawingCommandActive()) {
        commandInput_.append(character);
        updateOverlayText();
    }
}
void CadApplicationController::backspaceCommandBuffer() noexcept {
    if (polylineInput_.state() != arz::interaction::PolylineInputState::Inactive
        && !activeOptionBuffer_.empty()) {
        activeOptionBuffer_.pop_back();
        updateOverlayText();
    } else if (!anyDrawingCommandActive()) {
        commandInput_.backspace();
        updateOverlayText();
    }
}
void CadApplicationController::setCommandBuffer(std::string text) {
    if (!anyDrawingCommandActive()) {
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
    if (polylineInput_.state() != arz::interaction::PolylineInputState::Inactive) {
        const bool close = activeOptionBuffer_ == "C" || activeOptionBuffer_ == "CLOSE";
        activeOptionBuffer_.clear();
        if (close) return polylineInput_.canClose() && commitPolyline(true);
        if (polylineInput_.canFinish()) return commitPolyline(false);
        polylineInput_.cancel();
        overlayState_.clearDrawingPolyline();
        updateOverlayText();
        return true;
    }
    if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingRadiusPoint
        && overlayState_.drawingCircle()) {
        const auto preview = *overlayState_.drawingCircle();
        auto command = std::make_unique<arz::cad::AddCircleCommand>(
            document_, currentLayerId_, preview.center, preview.radius);
        auto* view = command.get();
        if (!history_.execute(std::move(command))) return false;
        circleInput_.cancel(); overlayState_.clearDrawingCircle();
        selection_.replace({view->objectId()}); synchronizeAfterModelChange(); return true;
    }
    if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingEnd
        && overlayState_.drawingArc()) {
        const auto preview = *overlayState_.drawingArc();
        auto command = std::make_unique<arz::cad::AddArcCommand>(document_, currentLayerId_,
            preview.center, preview.radius, preview.startAngle,
            arz::geometry::normalizeAngle(preview.startAngle
                + (preview.counterClockwise ? preview.sweepAngle : -preview.sweepAngle)),
            preview.counterClockwise);
        auto* view = command.get();
        if (!history_.execute(std::move(command))) return false;
        arcInput_.cancel(); overlayState_.clearDrawingArc();
        selection_.replace({view->objectId()}); synchronizeAfterModelChange(); return true;
    }
    if (lineInput_.state() != arz::interaction::LineInputState::Inactive) {
        lineInput_.cancel();
        updateOverlayText();
        return true;
    }
    if (anyDrawingCommandActive()) {
        cancelDrawingCommands();
        updateOverlayText();
        return true;
    }
    return commandInput_.lastRepeatable()
        && invokeCommandText(*commandInput_.lastRepeatable());
}

bool CadApplicationController::escape() {
    if (finishActiveInteraction()) return true;
    if (!commandInput_.buffer().empty()) {
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
bool CadApplicationController::rightClick() {
    if (polylineInput_.state() != arz::interaction::PolylineInputState::Inactive
        && polylineInput_.canFinish()) return commitPolyline(false);
    return finishActiveInteraction();
}
void CadApplicationController::selectPreviousSuggestion() noexcept {
    overlayState_.selectPreviousSuggestion();
}
void CadApplicationController::selectNextSuggestion() noexcept {
    overlayState_.selectNextSuggestion();
}

void CadApplicationController::updatePointer(arz::geometry::Point2D worldPoint,
                                             double worldTolerance) {
    if (overlayState_.selectionWindow()) overlayState_.updateSelectionWindow(worldPoint);
    if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
    if (polylineInput_.state() == arz::interaction::PolylineInputState::AwaitingNextPoint) {
        auto vertices = polylineInput_.vertices();
        vertices.push_back(worldPoint);
        overlayState_.setDrawingPolyline({std::move(vertices), false});
    }
    if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingRadiusPoint) {
        overlayState_.setDrawingCircle({*circleInput_.center(),
            arz::geometry::distance(*circleInput_.center(), worldPoint)});
    }
    if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingEnd) {
        if (const auto arc = arcInput_.preview(worldPoint)) {
            overlayState_.setDrawingArc({arc->center, arc->radius, arc->startAngle,
                arz::geometry::directedAngleSweep(arc->startAngle, arc->endAngle,
                                                   arc->counterClockwise),
                arc->counterClockwise});
        } else overlayState_.clearDrawingArc();
    }
    if (!overlayState_.pastePlacement()) return;
    const auto base = clipboard_.basePoint();
    const arz::geometry::Point2D delta{worldPoint.x - base.x, worldPoint.y - base.y};
    arz::interaction::PastePlacementOverlay placement;
    placement.basePoint = base;
    placement.insertionPoint = worldPoint;
    for (const auto& source : clipboard_.lines()) {
        placement.lines.push_back({
            {source.start.x + delta.x, source.start.y + delta.y},
            {source.end.x + delta.x, source.end.y + delta.y}
        });
    }
    for (const auto& source : clipboard_.polylines()) {
        auto vertices = source.vertices;
        for (auto& point : vertices) { point.x += delta.x; point.y += delta.y; }
        placement.polylines.push_back({std::move(vertices), source.closed});
    }
    for (const auto& source : clipboard_.circles())
        placement.circles.push_back({{source.center.x + delta.x, source.center.y + delta.y}, source.radius});
    for (const auto& source : clipboard_.arcs())
        placement.arcs.push_back({{source.center.x + delta.x, source.center.y + delta.y},
            source.radius, source.startAngle,
            arz::geometry::directedAngleSweep(source.startAngle, source.endAngle, source.counterClockwise),
            source.counterClockwise});
    overlayState_.setPastePlacement(std::move(placement));
}

bool CadApplicationController::finishActiveInteraction() {
    if (overlayState_.selectionWindow()) {
        overlayState_.clearSelectionWindow();
    } else if (overlayState_.pastePlacement()) {
        overlayState_.clearPastePlacement();
    } else if (lineInput_.state() != arz::interaction::LineInputState::Inactive) {
        lineInput_.cancel();
    } else if (anyDrawingCommandActive()) {
        cancelDrawingCommands();
    } else {
        return false;
    }
    updateOverlayText();
    return true;
}

bool CadApplicationController::invoke(const arz::interaction::CommandDescriptor& descriptor) {
    bool succeeded = false;
    switch (descriptor.command) {
    case arz::interaction::CadCommand::Line:
        overlayState_.clearPastePlacement();
        cancelDrawingCommands();
        lineInput_.activate();
        succeeded = true;
        break;
    case arz::interaction::CadCommand::Polyline:
        overlayState_.clearPastePlacement(); cancelDrawingCommands();
        polylineInput_.activate(); succeeded = true; break;
    case arz::interaction::CadCommand::Circle:
        overlayState_.clearPastePlacement(); cancelDrawingCommands();
        circleInput_.activate(); succeeded = true; break;
    case arz::interaction::CadCommand::Arc:
        overlayState_.clearPastePlacement(); cancelDrawingCommands();
        arcInput_.activate(); succeeded = true; break;
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
    if (polylineInput_.state() != arz::interaction::PolylineInputState::Inactive) {
        if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
        (void)polylineInput_.acceptPoint(worldPoint);
        overlayState_.setDrawingPolyline({polylineInput_.vertices(), false});
        updateOverlayText();
        return CanvasAction::FirstLinePointAccepted;
    }
    if (circleInput_.state() != arz::interaction::CircleInputState::Inactive) {
        if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
        const auto center = circleInput_.center();
        const auto radius = circleInput_.acceptPoint(worldPoint);
        if (!radius) {
            updateOverlayText();
            return CanvasAction::FirstLinePointAccepted;
        }
        overlayState_.clearDrawingCircle();
        auto command = std::make_unique<arz::cad::AddCircleCommand>(
            document_, currentLayerId_, *center, *radius);
        auto* view = command.get();
        if (!history_.execute(std::move(command))) return CanvasAction::None;
        selection_.replace({view->objectId()});
        synchronizeAfterModelChange();
        return CanvasAction::EntityCreated;
    }
    if (arcInput_.state() != arz::interaction::ArcInputState::Inactive) {
        if (const auto snap = snapCandidate(worldPoint, worldTolerance)) worldPoint = snap->point;
        const auto arc = arcInput_.acceptPoint(worldPoint);
        if (!arc) {
            updateOverlayText();
            return CanvasAction::FirstLinePointAccepted;
        }
        overlayState_.clearDrawingArc();
        auto command = std::make_unique<arz::cad::AddArcCommand>(document_, currentLayerId_,
            arc->center, arc->radius, arc->startAngle, arc->endAngle,
            arc->counterClockwise);
        auto* view = command.get();
        if (!history_.execute(std::move(command))) return CanvasAction::None;
        selection_.replace({view->objectId()});
        synchronizeAfterModelChange();
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
    if ((!anyDrawingCommandActive() && !overlayState_.pastePlacement())
        || !draftingSettings_.enabled(arz::interaction::DraftingToggle::ObjectSnap))
        return std::nullopt;
    return snapService_.bestSnap(worldPoint, worldTolerance, EnabledSnapTypes);
}

bool CadApplicationController::undo() {
    cancelDrawingCommands();
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
    cancelDrawingCommands();
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
    cancelDrawingCommands();
    commandInput_.clearBuffer();
    overlayState_.clearSelectionWindow();
    const auto base = clipboard_.basePoint();
    arz::interaction::PastePlacementOverlay placement;
    placement.basePoint = base;
    placement.insertionPoint = base;
    for (const auto& source : clipboard_.lines())
        placement.lines.push_back({source.start, source.end});
    for (const auto& source : clipboard_.polylines())
        placement.polylines.push_back({source.vertices, source.closed});
    for (const auto& source : clipboard_.circles())
        placement.circles.push_back({source.center, source.radius});
    for (const auto& source : clipboard_.arcs())
        placement.arcs.push_back({source.center, source.radius, source.startAngle,
            arz::geometry::directedAngleSweep(source.startAngle, source.endAngle,
                                               source.counterClockwise),
            source.counterClockwise});
    overlayState_.setPastePlacement(std::move(placement));
    updateOverlayText();
    return true;
}

bool CadApplicationController::commitPaste(arz::geometry::Point2D insertionPoint) {
    if (clipboard_.empty() || !overlayState_.pastePlacement()) return false;
    const auto base = clipboard_.basePoint();
    const arz::geometry::Point2D delta{insertionPoint.x - base.x, insertionPoint.y - base.y};
    std::vector<arz::cad::CadEntityCreationData> entities;
    for (const auto& source : clipboard_.lines()) {
        entities.emplace_back(arz::cad::LineCreationData{source.layerId,
            {source.start.x + delta.x, source.start.y + delta.y},
            {source.end.x + delta.x, source.end.y + delta.y}, source.graphics});
    }
    for (const auto& source : clipboard_.polylines()) {
        auto vertices = source.vertices;
        for (auto& point : vertices) { point.x += delta.x; point.y += delta.y; }
        entities.emplace_back(arz::cad::PolylineCreationData{
            source.layerId, std::move(vertices), source.closed, source.graphics});
    }
    for (const auto& source : clipboard_.circles())
        entities.emplace_back(arz::cad::CircleCreationData{source.layerId,
            {source.center.x + delta.x, source.center.y + delta.y},
            source.radius, source.graphics});
    for (const auto& source : clipboard_.arcs())
        entities.emplace_back(arz::cad::ArcCreationData{source.layerId,
            {source.center.x + delta.x, source.center.y + delta.y}, source.radius,
            source.startAngle, source.endAngle, source.counterClockwise, source.graphics});
    auto command = std::make_unique<arz::cad::AddCadEntitiesCommand>(
        document_, std::move(entities));
    auto* view = command.get();
    if (!history_.execute(std::move(command))) return false;
    selection_.replace(view->objectIds());
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
arz::interaction::PolylineInputState CadApplicationController::polylineInputState() const noexcept { return polylineInput_.state(); }
const std::vector<arz::geometry::Point2D>& CadApplicationController::polylineVertices() const noexcept { return polylineInput_.vertices(); }
arz::interaction::CircleInputState CadApplicationController::circleInputState() const noexcept { return circleInput_.state(); }
std::optional<arz::geometry::Point2D> CadApplicationController::circleCenter() const noexcept { return circleInput_.center(); }
arz::interaction::ArcInputState CadApplicationController::arcInputState() const noexcept { return arcInput_.state(); }
std::string CadApplicationController::commandPrompt() const {
    using arz::interaction::LineInputState;
    if (overlayState_.pastePlacement()) return "PASTE: Specify insertion point";
    if (overlayState_.selectionWindow()) return "Specify opposite corner";
    if (lineInput_.state() == LineInputState::AwaitingFirstPoint) return "LINE: Specify first point";
    if (lineInput_.state() == LineInputState::AwaitingSecondPoint) return "LINE: Specify next point";
    if (polylineInput_.state() == arz::interaction::PolylineInputState::AwaitingStartPoint) return "PLINE: Specify start point";
    if (polylineInput_.state() == arz::interaction::PolylineInputState::AwaitingNextPoint) return "PLINE: Specify next point or [Close]";
    if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingCenter) return "CIRCLE: Specify center point";
    if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingRadiusPoint) return "CIRCLE: Specify radius point";
    if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingStart) return "ARC: Specify start point";
    if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingSecond) return "ARC: Specify second point";
    if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingEnd) return "ARC: Specify end point";
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
    else if (polylineInput_.state() == arz::interaction::PolylineInputState::AwaitingStartPoint) overlayState_.setDynamicText("Specify start point");
    else if (polylineInput_.state() == arz::interaction::PolylineInputState::AwaitingNextPoint) overlayState_.setDynamicText(activeOptionBuffer_.empty() ? "Specify next point or [Close]" : activeOptionBuffer_);
    else if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingCenter) overlayState_.setDynamicText("Specify center point");
    else if (circleInput_.state() == arz::interaction::CircleInputState::AwaitingRadiusPoint) overlayState_.setDynamicText("Specify radius point");
    else if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingStart) overlayState_.setDynamicText("Specify start point");
    else if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingSecond) overlayState_.setDynamicText("Specify second point");
    else if (arcInput_.state() == arz::interaction::ArcInputState::AwaitingEnd) overlayState_.setDynamicText("Specify end point");
    else overlayState_.setDynamicText({});
}

bool CadApplicationController::commitPolyline(bool closed) {
    if ((!closed && !polylineInput_.canFinish()) || (closed && !polylineInput_.canClose()))
        return false;
    auto command = std::make_unique<arz::cad::AddPolylineCommand>(
        document_, currentLayerId_, polylineInput_.vertices(), closed);
    auto* view = command.get();
    if (!history_.execute(std::move(command))) return false;
    polylineInput_.cancel();
    activeOptionBuffer_.clear();
    overlayState_.clearDrawingPolyline();
    selection_.replace({view->objectId()});
    synchronizeAfterModelChange();
    return true;
}

bool CadApplicationController::anyDrawingCommandActive() const noexcept {
    return lineInput_.state() != arz::interaction::LineInputState::Inactive
        || polylineInput_.state() != arz::interaction::PolylineInputState::Inactive
        || circleInput_.state() != arz::interaction::CircleInputState::Inactive
        || arcInput_.state() != arz::interaction::ArcInputState::Inactive;
}

void CadApplicationController::cancelDrawingCommands() noexcept {
    lineInput_.cancel();
    polylineInput_.cancel();
    circleInput_.cancel();
    arcInput_.cancel();
    activeOptionBuffer_.clear();
    overlayState_.clearDrawingPolyline();
    overlayState_.clearDrawingCircle();
    overlayState_.clearDrawingArc();
}

}
