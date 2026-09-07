#include <cmath>
#include <cstdlib>
#include <iostream>

#include "app/application/CadApplicationController.h"
#include "interaction/commands/CommandLineParser.h"
#include "interaction/tools/LineInputController.h"
#include "rendering/Viewport2D.h"

namespace {

bool nearlyEqual(double first, double second) {
    return std::abs(first - second) <= 1e-9;
}

bool testViewportTransforms() {
    arz::rendering::Viewport2D viewport(0.1);
    viewport.setScreenOrigin({400.0, 300.0});
    const auto screen = viewport.worldToScreen({1000.0, 500.0});

    if (screen != arz::geometry::Point2D{500.0, 250.0}
        || viewport.screenToWorld(screen)
            != arz::geometry::Point2D{1000.0, 500.0}) {
        return false;
    }

    viewport.pan({25.0, -10.0});
    const auto anchor = arz::geometry::Point2D{525.0, 240.0};
    const auto worldBefore = viewport.screenToWorld(anchor);
    viewport.zoomAt(anchor, 2.0);
    const auto worldAfter = viewport.screenToWorld(anchor);

    return nearlyEqual(worldBefore.x, worldAfter.x)
        && nearlyEqual(worldBefore.y, worldAfter.y)
        && nearlyEqual(viewport.scale(), 0.2);
}

bool testCommandLineParser() {
    using arz::interaction::CommandLineParser;
    using arz::interaction::EditorCommand;

    return CommandLineParser::parse("LINE") == EditorCommand::Line
        && CommandLineParser::parse(" l ") == EditorCommand::Line
        && CommandLineParser::parse("undo") == EditorCommand::Undo
        && CommandLineParser::parse("REDO") == EditorCommand::Redo
        && CommandLineParser::parse("Esc") == EditorCommand::Cancel
        && CommandLineParser::parse("unknown") == EditorCommand::Unknown;
}

bool testLineInputStateMachine() {
    arz::interaction::LineInputController input;

    if (input.state() != arz::interaction::LineInputState::Inactive
        || input.acceptPoint({1.0, 2.0}).has_value()) {
        return false;
    }

    input.activate();

    if (input.acceptPoint({10.0, 20.0}).has_value()
        || input.state()
            != arz::interaction::LineInputState::AwaitingSecondPoint
        || input.firstPoint()
            != arz::geometry::Point2D{10.0, 20.0}) {
        return false;
    }

    const auto line = input.acceptPoint({30.0, 40.0});

    if (!line
        || line->start != arz::geometry::Point2D{10.0, 20.0}
        || line->end != arz::geometry::Point2D{30.0, 40.0}
        || input.state() != arz::interaction::LineInputState::Inactive) {
        return false;
    }

    input.activate();
    const auto pending = input.acceptPoint({5.0, 6.0});
    (void)pending;
    input.cancel();
    return input.state() == arz::interaction::LineInputState::Inactive
        && !input.firstPoint().has_value();
}

bool testControllerAddUndoRedoSpatialSynchronization() {
    arz::app::CadApplicationController controller;
    controller.startLine();

    if (controller.canvasClick({0.0, 0.0}, 5.0)
            != arz::app::CanvasAction::FirstLinePointAccepted
        || controller.canvasClick({1000.0, 0.0}, 5.0)
            != arz::app::CanvasAction::EntityCreated
        || controller.document().objectCount() != 1
        || controller.spatialIndex().size() != 1
        || !controller.history().canUndo()) {
        return false;
    }

    const auto objectId = controller.selectedObjectId();

    if (objectId == arz::core::InvalidObjectId
        || !controller.undo()
        || controller.document().objectCount() != 0
        || controller.spatialIndex().size() != 0
        || !controller.history().canRedo()) {
        return false;
    }

    if (!controller.redo()
        || controller.document().objectCount() != 1
        || controller.spatialIndex().size() != 1
        || !controller.document().contains(objectId)) {
        return false;
    }

    controller.startLine();
    const auto snapClick = controller.canvasClick({2.0, 1.0}, 5.0);
    (void)snapClick;

    return controller.lineStartPoint()
        == arz::geometry::Point2D{0.0, 0.0};
}

}

int main() {
    int failures = 0;
    const auto run = [&failures](const char* name, bool result) {
        if (result) {
            std::cout << "[PASS] " << name << '\n';
        } else {
            std::cerr << "[FAIL] " << name << '\n';
            ++failures;
        }
    };

    run("ViewportTransforms", testViewportTransforms());
    run("CommandLineParser", testCommandLineParser());
    run("LineInputStateMachine", testLineInputStateMachine());
    run(
        "ControllerAddUndoRedoSpatialSynchronization",
        testControllerAddUndoRedoSpatialSynchronization()
    );

    if (failures != 0) {
        std::cerr << failures << " interactive foundation test(s) failed.\n";
        return EXIT_FAILURE;
    }

    std::cout << "All interactive foundation tests passed.\n";
    return EXIT_SUCCESS;
}
