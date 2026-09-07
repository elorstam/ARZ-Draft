#include <cstdlib>
#include <iostream>
#include <string_view>

#include "app/application/CadApplicationController.h"
#include "interaction/commands/CommandInputState.h"
#include "interaction/commands/CommandRegistry.h"
#include "interaction/drafting/DraftingSettings.h"
#include "interaction/selection/SelectionSet.h"

namespace {

bool addLine(arz::app::CadApplicationController& controller,
             arz::geometry::Point2D start,
             arz::geometry::Point2D end) {
    controller.startLine();
    return controller.canvasClick(start, 0.01)
            == arz::app::CanvasAction::FirstLinePointAccepted
        && controller.canvasClick(end, 0.01)
            == arz::app::CanvasAction::EntityCreated;
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
    if (!controller.selection().empty()) return false;

    controller.beginSelectionWindow({0, 0});
    controller.updateSelectionWindow({100, 100});
    if (!controller.finishSelectionWindow(false) || controller.selection().size() != 1) return false;
    controller.beginSelectionWindow({100, 100});
    controller.updateSelectionWindow({0, 0});
    if (!controller.finishSelectionWindow(false) || controller.selection().size() != 2) return false;
    controller.beginSelectionWindow({100, 100});
    controller.updateSelectionWindow({0, 0});
    if (!controller.finishSelectionWindow(true) || !controller.selection().empty()) return false;
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
    if (!controller.paste() || controller.document().objectCount() != 4
        || controller.selection().size() != 2) return false;
    for (const auto id : controller.selection().ids()) {
        if (std::ranges::find(originalIds, id) != originalIds.end()) return false;
    }
    if (!controller.undo() || controller.document().objectCount() != 2
        || !controller.redo() || controller.document().objectCount() != 4) return false;
    if (!controller.selectAll()) return false;
    if (!controller.cutSelection() || controller.document().objectCount() != 0) return false;
    return controller.undo() && controller.document().objectCount() == 4;
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
    run("EscapePriority", testEscapePriority());
    run("SelectionSetAndWindows", testSelectionSetAndWindows());
    run("DeleteClipboardTransactions", testDeleteClipboardTransactions());
    run("DraftingStateAndStaleCleanup", testDraftingStateAndStaleCleanup());
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
