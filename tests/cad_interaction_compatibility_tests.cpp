#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#include "app/application/CadApplicationController.h"
#include "cad/entities/LineEntity.h"
#include "interaction/commands/CommandInputPolicy.h"
#include "interaction/point/PointAcquisition.h"

namespace {

bool testSharedPointAcquisition() {
    arz::interaction::PointAcquisition acquisition;
    const auto raw = acquisition.resolve({{13, 7}, std::nullopt, std::nullopt, false});
    if (raw.point != arz::geometry::Point2D{13, 7} || raw.snap
        || raw.constrainedPoint || raw.orthoConstrained) return false;

    const arz::cad::SnapResult snap{
        42, arz::cad::SnapType::Endpoint, {10, 10}, 1.0};
    const auto snapped = acquisition.resolve({
        {11, 9}, snap, arz::geometry::Point2D{0, 0}, true});
    if (snapped.point != snap.point || !snapped.snap
        || snapped.snap->objectId != snap.objectId
        || snapped.snap->type != snap.type
        || snapped.snappedPoint != snap.point || snapped.orthoConstrained) return false;

    const auto horizontal = acquisition.resolve({
        {15, 4}, std::nullopt, arz::geometry::Point2D{2, 3}, true});
    const auto vertical = acquisition.resolve({
        {4, 20}, std::nullopt, arz::geometry::Point2D{2, 3}, true});
    const auto deterministicTie = acquisition.resolve({
        {12, 13}, std::nullopt, arz::geometry::Point2D{2, 3}, true});
    return horizontal.point == arz::geometry::Point2D{15, 3}
        && vertical.point == arz::geometry::Point2D{2, 20}
        && deterministicTie.point == arz::geometry::Point2D{12, 3};
}

bool testLineChainingAndFinishSemantics() {
    using arz::interaction::LineInputState;
    arz::app::CadApplicationController controller;
    controller.startLine();
    if (controller.canvasClick({0, 0}, 0.01)
            != arz::app::CanvasAction::FirstLinePointAccepted) return false;
    controller.updatePointer({10, 0}, 0.01);
    if (!controller.overlayState().drawingLine()
        || controller.overlayState().drawingLine()->end != arz::geometry::Point2D{10, 0}
        || controller.overlayState().dynamicInput().distance != 10.0
        || controller.overlayState().dynamicInput().angleDegrees != 0.0
        || controller.overlayState().dynamicInput().options
            != std::vector<std::string>{"Undo"}
        || controller.document().objectCount() != 0) return false;
    if (controller.canvasClick({10, 0}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.lineInputState() != LineInputState::AwaitingSecondPoint
        || controller.lineStartPoint() != arz::geometry::Point2D{10, 0}) return false;
    controller.updatePointer({10, 10}, 0.01);
    if (controller.canvasClick({10, 10}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 2
        || controller.history().undoCount() != 2) return false;
    const auto ids = controller.document().objectIds();
    const auto* first = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[0]));
    const auto* second = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[1]));
    if (!first || !second || first->end() != second->start()
        || second->end() != arz::geometry::Point2D{10, 10}) return false;
    controller.appendCommandCharacter('u');
    if (!controller.confirmInput() || controller.document().objectCount() != 1
        || controller.lineStartPoint() != arz::geometry::Point2D{10, 0}) return false;
    if (controller.canvasClick({10, 10}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 2) return false;
    if (!controller.confirmInput() || controller.lineInputState() != LineInputState::Inactive
        || controller.overlayState().drawingLine()) return false;

    if (!controller.confirmInput() || controller.lineInputState() != LineInputState::AwaitingFirstPoint
        || !controller.confirmInput() || controller.lineInputState() != LineInputState::Inactive)
        return false;
    if (!controller.confirmInput() || controller.lineInputState() != LineInputState::AwaitingFirstPoint)
        return false;
    (void)controller.canvasClick({20, 20}, 0.01);
    (void)controller.canvasClick({30, 20}, 0.01);
    return controller.escape() && controller.lineInputState() == LineInputState::Inactive
        && controller.document().objectCount() == 3;
}

bool testPreviewResolutionAndNoMutation() {
    using arz::interaction::DraftingToggle;
    arz::app::CadApplicationController controller;
    controller.startLine();
    (void)controller.canvasClick({0, 0}, 0.01);
    (void)controller.canvasClick({100, 0}, 0.01);
    (void)controller.confirmInput();

    controller.startLine();
    (void)controller.canvasClick({25, 25}, 0.01);
    const auto baselineObjects = controller.document().objectCount();
    const auto baselineUndo = controller.history().undoCount();
    for (int index = 0; index < 500; ++index)
        controller.updatePointer({200.0 + index, 80.0 + index * 0.25}, 2.0);
    if (controller.document().objectCount() != baselineObjects
        || controller.history().undoCount() != baselineUndo) return false;

    controller.updatePointer({99, 1}, 5.0);
    auto preview = controller.overlayState().resolvedPoint();
    if (!preview || preview->point != arz::geometry::Point2D{100, 0} || !preview->snap)
        return false;
    (void)controller.toggleDrafting(DraftingToggle::ObjectSnap);
    preview = controller.overlayState().resolvedPoint();
    if (!preview || preview->point != arz::geometry::Point2D{99, 1} || preview->snap)
        return false;
    (void)controller.toggleDrafting(DraftingToggle::Ortho);
    preview = controller.overlayState().resolvedPoint();
    if (!preview || preview->point != arz::geometry::Point2D{99, 25}
        || !preview->orthoConstrained) return false;
    const auto committedPoint = preview->point;
    if (controller.canvasClick({99, 1}, 5.0) != arz::app::CanvasAction::EntityCreated)
        return false;
    const auto* line = dynamic_cast<const arz::cad::LineEntity*>(
        controller.document().object(controller.selectedObjectId()));
    return line && line->end() == committedPoint
        && controller.overlayState().drawingLine()
        && controller.overlayState().drawingLine()->start == committedPoint;
}

bool testRightClickAndRepeatPolicy() {
    using arz::interaction::CommandInputPolicy;
    using arz::interaction::InteractionStage;
    using arz::interaction::RightClickAction;
    if (CommandInputPolicy::rightClick(InteractionStage::Idle)
            != RightClickAction::ShowContextMenu
        || CommandInputPolicy::rightClick(InteractionStage::SelectionActive)
            != RightClickAction::ShowContextMenu
        || CommandInputPolicy::rightClick(InteractionStage::AwaitingPoint)
            != RightClickAction::FinishInteraction) return false;

    arz::app::CadApplicationController controller;
    if (controller.interactionStage() != InteractionStage::Idle || controller.rightClick())
        return false;
    if (controller.canvasClick({500, 500}, 0.01)
            != arz::app::CanvasAction::SelectionWindowStarted
        || controller.interactionStage() != InteractionStage::AwaitingSelectionConfirmation
        || !controller.confirmInput()
        || controller.interactionStage() != InteractionStage::Idle) return false;
    controller.startLine();
    if (controller.interactionStage() != InteractionStage::AwaitingPoint
        || !controller.rightClick()
        || controller.interactionStage() != InteractionStage::Idle) return false;
    return controller.confirmInput()
        && controller.lineInputState() == arz::interaction::LineInputState::AwaitingFirstPoint;
}

}

int main() {
    int failures = 0;
    const auto run = [&failures](std::string_view name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
        if (!passed) ++failures;
    };
    run("SharedPointAcquisition", testSharedPointAcquisition());
    run("LineChainingAndFinishSemantics", testLineChainingAndFinishSemantics());
    run("PreviewResolutionAndNoMutation", testPreviewResolutionAndNoMutation());
    run("RightClickAndRepeatPolicy", testRightClickAndRepeatPolicy());
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
