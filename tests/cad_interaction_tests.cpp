#include <cstdlib>
#include <iostream>
#include <string_view>

#include "app/application/CadApplicationController.h"
#include "cad/entities/LineEntity.h"
#include "interaction/commands/CommandInputState.h"
#include "interaction/commands/CommandRegistry.h"
#include "interaction/drafting/DraftingSettings.h"
#include "interaction/selection/SelectionSet.h"

namespace {

bool addLine(arz::app::CadApplicationController& controller,
             arz::geometry::Point2D start,
             arz::geometry::Point2D end) {
    controller.startLine();
    const bool created = controller.canvasClick(start, 0.01)
            == arz::app::CanvasAction::FirstLinePointAccepted
        && controller.canvasClick(end, 0.01)
            == arz::app::CanvasAction::EntityCreated;
    return created && controller.confirmInput();
}

bool testCommandInputContract() {
    arz::app::CadApplicationController controller;
    for (const char value : std::string("LINX")) controller.appendCommandCharacter(value);
    controller.backspaceCommandBuffer();
    controller.appendCommandCharacter('e');
    if (controller.commandInput().buffer() != "LINE"
        || controller.overlayState().dynamicText() != "LINE"
        || !controller.confirmInput()
        || controller.lineInputState() != arz::interaction::LineInputState::AwaitingFirstPoint
        || controller.commandInput().history() != std::vector<std::string>{"LINE"}) return false;
    if (!controller.confirmInput()) return false;
    (void)controller.canvasClick({0, 0}, 0.01);
    if (!controller.escape() || controller.lineInputState() != arz::interaction::LineInputState::Inactive)
        return false;
    if (!controller.confirmInput()
        || controller.commandInput().history() != std::vector<std::string>({"LINE", "LINE"})) return false;
    (void)controller.escape();
    controller.appendCommandCharacter('L');
    return controller.confirmInput()
        && controller.commandInput().history().back() == "LINE";
}

bool testCommandSuggestions() {
    using arz::interaction::CadCommand;
    using arz::interaction::CommandDescriptor;
    arz::interaction::CommandRegistry registry({
        {CadCommand::Line, "LINE", {"L"}, true},
        {CadCommand::Unknown, "LAYER", {"LA"}, false},
        {CadCommand::Unknown, "LENGTHEN", {"LEN"}, false},
        {CadCommand::Unknown, "LINETYPE", {"LT"}, false}
    });
    const auto lower = registry.suggest("l");
    if (lower.size() != 4 || lower[0].canonicalName != "LINE"
        || lower[1].canonicalName != "LAYER"
        || lower[2].canonicalName != "LENGTHEN"
        || lower[3].canonicalName != "LINETYPE") return false;
    const auto filtered = registry.suggest("len");
    if (filtered.size() != 1 || filtered.front().canonicalName != "LENGTHEN") return false;
    const auto alias = registry.suggest("la");
    if (alias.empty() || alias.front().canonicalName != "LAYER") return false;

    arz::app::CadApplicationController controller;
    controller.appendCommandCharacter('l');
    return controller.overlayState().commandSuggestions()
            == std::vector<std::string>{"LINE"}
        && controller.overlayState().selectedSuggestionIndex() == 0
        && controller.confirmInput()
        && controller.lineInputState() == arz::interaction::LineInputState::AwaitingFirstPoint;
}

bool testEscapePriority() {
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {0, 0}, {100, 0})) return false;
    controller.beginSelectionWindow({0, 0});
    controller.updateSelectionWindow({100, 100});
    if (!controller.escape() || controller.overlayState().selectionWindow()) return false;
    controller.startLine();
    controller.beginSelectionWindow({0, 0});
    if (!controller.escape() || controller.lineInputState() != arz::interaction::LineInputState::Inactive)
        return false;
    controller.appendCommandCharacter('L');
    if (!controller.escape() || !controller.commandInput().buffer().empty()) return false;
    (void)controller.canvasClick({50, 0}, 1.0);
    return !controller.selection().empty() && controller.escape()
        && controller.selection().empty() && !controller.escape();
}

bool testSelectionSetAndWindows() {
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {10, 10}, {20, 20})
        || !addLine(controller, {-50, 50}, {50, 50})
        || !addLine(controller, {200, 200}, {300, 300})) return false;
    (void)controller.escape();
    (void)controller.canvasClick({15, 15}, 1.0);
    (void)controller.canvasClick({250, 250}, 1.0);
    if (controller.selection().size() != 2) return false;
    (void)controller.canvasClick({15, 15}, 1.0, true);
    if (controller.selection().size() != 1) return false;
    (void)controller.canvasClick({500, 500}, 1.0);
    if (!controller.selection().empty() || !controller.escape()) return false;

    if (controller.canvasClick({0, 0}, 0.1) != arz::app::CanvasAction::SelectionWindowStarted
        || !controller.overlayState().selectionWindow()) return false;
    controller.updatePointer({100, 100});
    if (controller.overlayState().selectionWindow()->current != arz::geometry::Point2D{100, 100}
        || controller.canvasClick({100, 100}, 0.1) != arz::app::CanvasAction::SelectionChanged
        || controller.selection().size() != 1) return false;
    if (controller.canvasClick({100, 100}, 0.1) != arz::app::CanvasAction::SelectionWindowStarted)
        return false;
    controller.updatePointer({0, 0});
    if (controller.canvasClick({0, 0}, 0.1) != arz::app::CanvasAction::SelectionChanged
        || controller.selection().size() != 2) return false;
    if (controller.canvasClick({100, 100}, 0.1, true) != arz::app::CanvasAction::SelectionWindowStarted)
        return false;
    controller.updatePointer({0, 0});
    (void)controller.canvasClick({0, 0}, 0.1, true);
    if (!controller.selection().empty()) return false;
    (void)controller.canvasClick({500, 500}, 0.1);
    if (!controller.overlayState().selectionWindow() || !controller.escape()
        || controller.overlayState().selectionWindow()) return false;
    (void)controller.canvasClick({15, 15}, 1.0);
    if (controller.overlayState().selectionWindow()) return false;
    return controller.selectAll() && controller.selection().size() == 3;
}

bool testDeleteClipboardTransactions() {
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {0, 0}, {100, 0})
        || !addLine(controller, {0, 100}, {100, 100})
        || !controller.selectAll()) return false;
    const auto originalIds = controller.selection().ids();
    const auto beforeDelete = controller.history().undoCount();
    if (!controller.copySelection() || controller.clipboard().lines().size() != 2
        || !controller.deleteSelection()
        || controller.history().undoCount() != beforeDelete + 1
        || controller.document().objectCount() != 0) return false;
    if (!controller.undo() || controller.document().objectCount() != 2) return false;
    if (!controller.redo() || controller.document().objectCount() != 0 || !controller.undo()) return false;
    if (!controller.paste() || !controller.pastePlacementActive()
        || controller.document().objectCount() != 2) return false;
    const auto* source = dynamic_cast<const arz::cad::LineEntity*>(
        controller.document().object(originalIds.front()));
    if (!source) return false;
    const auto sourceStart = source->start();
    const auto sourceEnd = source->end();
    for (int index = 0; index < 100; ++index)
        controller.updatePointer({1000.0 + index * 3.0, 500.0 - index * 2.0});
    if (controller.document().objectCount() != 2
        || controller.overlayState().pastePlacement()->lines.size() != 2
        || source->start() != sourceStart || source->end() != sourceEnd) return false;
    const auto preview = controller.overlayState().pastePlacement()->lines.front();
    const auto insertion = controller.overlayState().pastePlacement()->insertionPoint;
    const auto base = controller.clipboard().basePoint();
    if (preview.start != arz::geometry::Point2D{
            sourceStart.x + insertion.x - base.x,
            sourceStart.y + insertion.y - base.y}) return false;
    if (controller.canvasClick(insertion, 0.1) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 4
        || controller.selection().size() != 2 || controller.pastePlacementActive()) return false;
    for (const auto id : controller.selection().ids()) {
        if (std::ranges::find(originalIds, id) != originalIds.end()) return false;
    }
    if (!controller.undo() || controller.document().objectCount() != 2
        || !controller.redo() || controller.document().objectCount() != 4) return false;
    if (!controller.selectAll() || !controller.copySelection() || !controller.paste()) return false;
    for (int index = 0; index < 50; ++index) controller.updatePointer({-index * 4.0, index * 7.0});
    if (controller.document().objectCount() != 4
        || controller.overlayState().pastePlacement()->lines.size() != 4) return false;
    const auto secondInsertion = controller.overlayState().pastePlacement()->insertionPoint;
    if (controller.canvasClick(secondInsertion, 0.1) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 8) return false;
    if (!controller.selectAll()) return false;
    if (!controller.cutSelection() || controller.document().objectCount() != 0) return false;
    return controller.undo() && controller.document().objectCount() == 8;
}

bool testPasteCancelDoesNotMutateDocument() {
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {0, 0}, {100, 25}) || !controller.copySelection()) return false;
    if (!controller.paste()) return false;
    for (int index = 0; index < 20; ++index) controller.updatePointer({index * 10.0, index * 5.0});
    return controller.document().objectCount() == 1 && controller.escape()
        && !controller.pastePlacementActive() && controller.document().objectCount() == 1;
}

bool testSingleLinePasteRegression() {
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {25, 40}, {125, 65}) || !controller.copySelection()) return false;
    const auto sourceId = controller.selectedObjectId();
    const auto* source = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(sourceId));
    if (!source) return false;
    const auto sourceStart = source->start();
    const auto sourceEnd = source->end();
    if (!controller.paste()) return false;
    for (int repeat = 0; repeat < 20; ++repeat) {
        if (!controller.paste() || controller.document().objectCount() != 1) return false;
    }
    for (int index = 0; index < 200; ++index)
        controller.updatePointer({500.0 - index * 2.0, -300.0 + index * 1.5});
    if (controller.document().objectCount() != 1
        || controller.overlayState().pastePlacement()->lines.size() != 1) return false;
    const auto insertion = controller.overlayState().pastePlacement()->insertionPoint;
    if (controller.canvasClick(insertion, 0.1) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 2
        || controller.selectedObjectId() == sourceId
        || source->start() != sourceStart || source->end() != sourceEnd) return false;
    const auto pastedId = controller.selectedObjectId();
    if (!controller.undo() || controller.document().objectCount() != 1
        || controller.document().contains(pastedId)) return false;
    return controller.redo() && controller.document().objectCount() == 2
        && controller.document().contains(pastedId);
}

bool testPastePlacementObjectSnap() {
    using arz::interaction::DraftingToggle;
    arz::app::CadApplicationController controller;
    if (!addLine(controller, {0, 0}, {100, 0})
        || !addLine(controller, {1000, 1000}, {1020, 1010})
        || !controller.copySelection()) return false;

    const auto sourceId = controller.selectedObjectId();
    const auto sourceSnapshot = controller.clipboard().lines().front();
    const auto base = controller.clipboard().basePoint();
    const auto initialCount = controller.document().objectCount();
    if (!controller.paste()) return false;

    for (int index = 0; index < 100; ++index) {
        controller.updatePointer({400.0 + index, 300.0 - index}, 5.0);
    }
    if (controller.document().objectCount() != initialCount
        || controller.clipboard().lines().front().start != sourceSnapshot.start
        || controller.clipboard().lines().front().end != sourceSnapshot.end) return false;

    controller.updatePointer({2, 1}, 5.0);
    const auto endpointPlacement = controller.overlayState().pastePlacement();
    if (!endpointPlacement
        || endpointPlacement->insertionPoint != arz::geometry::Point2D{0, 0}
        || endpointPlacement->lines.front().start
            != arz::geometry::Point2D{sourceSnapshot.start.x - base.x,
                                      sourceSnapshot.start.y - base.y}) return false;
    const auto endpointPreview = endpointPlacement->lines.front();
    if (controller.canvasClick({2, 1}, 5.0) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != initialCount + 1
        || controller.selectedObjectId() == sourceId) return false;
    const auto* endpointPaste = dynamic_cast<const arz::cad::LineEntity*>(
        controller.document().object(controller.selectedObjectId()));
    if (!endpointPaste || endpointPaste->start() != endpointPreview.start
        || endpointPaste->end() != endpointPreview.end) return false;

    if (!controller.paste()) return false;
    controller.updatePointer({51, 1}, 5.0);
    const auto midpointPlacement = controller.overlayState().pastePlacement();
    if (!midpointPlacement
        || midpointPlacement->insertionPoint != arz::geometry::Point2D{50, 0}) return false;
    if (!controller.escape()) return false;

    (void)controller.toggleDrafting(DraftingToggle::ObjectSnap);
    if (!controller.paste()) return false;
    const arz::geometry::Point2D rawPoint{2, 1};
    controller.updatePointer(rawPoint, 5.0);
    const auto rawPlacement = controller.overlayState().pastePlacement();
    return rawPlacement && rawPlacement->insertionPoint == rawPoint
        && controller.clipboard().lines().front().start == sourceSnapshot.start
        && controller.clipboard().lines().front().end == sourceSnapshot.end
        && controller.escape()
        && controller.document().objectCount() == initialCount + 1;
}

bool testContinuousLineContract() {
    using arz::interaction::LineInputState;
    arz::app::CadApplicationController controller;
    controller.startLine();
    if (controller.canvasClick({0, 0}, 0.01)
            != arz::app::CanvasAction::FirstLinePointAccepted
        || controller.canvasClick({10, 0}, 0.01)
            != arz::app::CanvasAction::EntityCreated
        || controller.lineInputState() != LineInputState::AwaitingSecondPoint
        || controller.commandPrompt() != "LINE: Specify next point"
        || controller.canvasClick({10, 10}, 0.01)
            != arz::app::CanvasAction::EntityCreated
        || controller.canvasClick({20, 10}, 0.01)
            != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 3) return false;

    const auto ids = controller.document().objectIds();
    if (ids.size() != 3 || ids[0] == ids[1] || ids[1] == ids[2] || ids[0] == ids[2])
        return false;
    const auto* first = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[0]));
    const auto* second = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[1]));
    const auto* third = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[2]));
    if (!first || !second || !third
        || first->start() != arz::geometry::Point2D{0, 0}
        || first->end() != second->start()
        || second->end() != third->start()
        || third->end() != arz::geometry::Point2D{20, 10}) return false;

    if (!controller.confirmInput()
        || controller.lineInputState() != LineInputState::Inactive
        || !controller.confirmInput()
        || controller.lineInputState() != LineInputState::AwaitingFirstPoint
        || !controller.confirmInput()
        || controller.lineInputState() != LineInputState::Inactive) return false;

    controller.startLine();
    if (!controller.escape() || controller.lineInputState() != LineInputState::Inactive)
        return false;

    controller.startLine();
    if (controller.canvasClick({200, 200}, 5.0)
            != arz::app::CanvasAction::FirstLinePointAccepted
        || controller.canvasClick({1, 1}, 5.0)
            != arz::app::CanvasAction::EntityCreated) return false;
    const auto* snapped = dynamic_cast<const arz::cad::LineEntity*>(
        controller.document().object(controller.selectedObjectId()));
    return snapped && snapped->start() == arz::geometry::Point2D{200, 200}
        && snapped->end() == arz::geometry::Point2D{0, 0}
        && controller.lineInputState() == LineInputState::AwaitingSecondPoint
        && controller.confirmInput()
        && controller.lineInputState() == LineInputState::Inactive;
}

bool testSuggestionNavigationState() {
    arz::interaction::OverlayState overlay;
    overlay.setCommandSuggestions({"LINE", "LINETYPE", "LENGTHEN"});
    if (overlay.selectedSuggestion() != "LINE") return false;
    overlay.selectNextSuggestion();
    if (overlay.selectedSuggestion() != "LINETYPE") return false;
    overlay.selectPreviousSuggestion();
    if (overlay.selectedSuggestion() != "LINE") return false;
    overlay.selectPreviousSuggestion();
    return overlay.selectedSuggestion() == "LENGTHEN";
}

bool testDraftingStateAndStaleCleanup() {
    arz::app::CadApplicationController controller;
    using arz::interaction::DraftingToggle;
    if (!controller.draftingSettings().enabled(DraftingToggle::ObjectSnap)
        || !controller.draftingSettings().enabled(DraftingToggle::DynamicInput)) return false;
    (void)controller.toggleDrafting(DraftingToggle::Ortho);
    (void)controller.toggleDrafting(DraftingToggle::PolarTracking);
    if (controller.draftingSettings().enabled(DraftingToggle::Ortho)
        || !controller.draftingSettings().enabled(DraftingToggle::PolarTracking)) return false;
    if (!addLine(controller, {0, 0}, {100, 0})) return false;
    const auto selected = controller.selectedObjectId();
    if (!controller.undo() || controller.document().contains(selected)
        || !controller.selection().empty()) return false;
    if (!controller.redo()) return false;
    controller.startLine();
    if (!controller.snapCandidate({1, 1}, 5.0)) return false;
    (void)controller.toggleDrafting(DraftingToggle::ObjectSnap);
    if (controller.snapCandidate({1, 1}, 5.0)) return false;
    (void)controller.toggleDrafting(DraftingToggle::DynamicInput);
    return controller.overlayState().dynamicText().empty();
}

}

int main() {
    int failures = 0;
    const auto run = [&failures](std::string_view name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
        if (!passed) ++failures;
    };
    run("CommandInputContract", testCommandInputContract());
    run("CommandSuggestions", testCommandSuggestions());
    run("EscapePriority", testEscapePriority());
    run("SelectionSetAndWindows", testSelectionSetAndWindows());
    run("DeleteClipboardTransactions", testDeleteClipboardTransactions());
    run("PasteCancelDoesNotMutateDocument", testPasteCancelDoesNotMutateDocument());
    run("SingleLinePasteRegression", testSingleLinePasteRegression());
    run("PastePlacementObjectSnap", testPastePlacementObjectSnap());
    run("ContinuousLineContract", testContinuousLineContract());
    run("SuggestionNavigationState", testSuggestionNavigationState());
    run("DraftingStateAndStaleCleanup", testDraftingStateAndStaleCleanup());
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
