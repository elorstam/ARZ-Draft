#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numbers>
#include <string_view>
#include <variant>
#include <vector>

#include "app/application/CadApplicationController.h"
#include "cad/commands/AddArcCommand.h"
#include "cad/commands/AddCircleCommand.h"
#include "cad/commands/AddPolylineCommand.h"
#include "cad/entities/ArcEntity.h"
#include "cad/entities/LineEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "cad/selection/CadEntityPickRefiner.h"
#include "cad/snapping/CadEntitySnapProvider.h"
#include "core/document/Document.h"
#include "core/transactions/TransactionHistory.h"
#include "geometry/algorithms/Curve2D.h"
#include "interaction/commands/CommandLineParser.h"
#include "interaction/commands/CommandRegistry.h"
#include "rendering/DocumentRenderPlanner.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"
#include "rendering/Viewport2D.h"

namespace {

bool near(double a, double b, double tolerance = 1e-6) {
    return std::abs(a - b) <= tolerance;
}

bool testEntitiesAndUndoRedoCommands() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    auto polyline = std::make_unique<arz::cad::AddPolylineCommand>(document,
        arz::cad::DefaultLayerId,
        std::vector<arz::geometry::Point2D>{{0, 0}, {30, 0}, {30, 40}}, false);
    auto* polylineCommand = polyline.get();
    if (!history.execute(std::move(polyline))) return false;
    const auto* value = dynamic_cast<const arz::cad::PolylineEntity*>(
        document.object(polylineCommand->objectId()));
    const auto polylineBounds = value ? value->boundingBox() : arz::geometry::BoundingBox2D{};
    if (!value || !near(value->length(), 70.0)
        || !near(polylineBounds.minX, 0.0) || !near(polylineBounds.minY, 0.0)
        || !near(polylineBounds.maxX, 30.0) || !near(polylineBounds.maxY, 40.0)) return false;

    auto circle = std::make_unique<arz::cad::AddCircleCommand>(
        document, arz::cad::DefaultLayerId, arz::geometry::Point2D{10, 20}, 5.0);
    auto* circleCommand = circle.get();
    if (!history.execute(std::move(circle))) return false;
    const auto* circleValue = dynamic_cast<const arz::cad::CircleEntity*>(
        document.object(circleCommand->objectId()));
    if (!circleValue || !near(circleValue->diameter(), 10.0)
        || !near(circleValue->area(), 25.0 * std::numbers::pi)) return false;

    auto arc = std::make_unique<arz::cad::AddArcCommand>(document,
        arz::cad::DefaultLayerId, arz::geometry::Point2D{0, 0}, 10.0,
        0.0, std::numbers::pi, true);
    auto* arcCommand = arc.get();
    if (!history.execute(std::move(arc))) return false;
    const auto* arcValue = dynamic_cast<const arz::cad::ArcEntity*>(
        document.object(arcCommand->objectId()));
    if (!arcValue || !near(arcValue->length(), 10.0 * std::numbers::pi)
        || !near(arcValue->boundingBox().maxY, 10.0)) return false;
    const auto arcId = arcCommand->objectId();
    if (!history.undo() || document.contains(arcId)
        || !history.redo() || !document.contains(arcId)) return false;
    return true;
}

bool testSelectionAndSnappingCoverage() {
    arz::cad::CadEntityPickRefiner refiner;
    const arz::cad::PolylineEntity polyline(1, arz::cad::DefaultLayerId,
        {{0, 0}, {20, 0}, {20, 20}}, false);
    const arz::cad::CircleEntity circle(2, arz::cad::DefaultLayerId, {50, 50}, 10);
    const arz::cad::ArcEntity arc(3, arz::cad::DefaultLayerId, {0, 0}, 10,
        0, std::numbers::pi, true);
    if (!refiner.hit(polyline, {10, 0.2}, 0.3)
        || refiner.hit(circle, {50, 50}, 1.0)
        || !refiner.hit(circle, {60, 50}, 0.01)
        || !refiner.hit(arc, {0, 10}, 0.01)
        || !refiner.intersects(circle, {55, 45, 65, 55})
        || !refiner.contained(polyline, {-1, -1, 21, 21})) return false;

    arz::cad::CadEntitySnapProvider provider;
    std::vector<arz::cad::SnapPoint> points;
    const std::vector circleTypes{arz::cad::SnapType::Center,
                                  arz::cad::SnapType::Quadrant};
    provider.appendSnapPoints(circle, circleTypes, points);
    if (points.size() != 5 || points.front().type != arz::cad::SnapType::Center)
        return false;
    points.clear();
    const std::vector arcTypes{arz::cad::SnapType::Endpoint,
                               arz::cad::SnapType::Midpoint,
                               arz::cad::SnapType::Center,
                               arz::cad::SnapType::Quadrant};
    provider.appendSnapPoints(arc, arcTypes, points);
    return points.size() >= 6;
}

bool testInteractiveCommandsPreviewsAndClipboard() {
    arz::app::CadApplicationController controller;
    if (!controller.invokeCommandText("pl")
        || controller.polylineInputState()
            != arz::interaction::PolylineInputState::AwaitingStartPoint) return false;
    (void)controller.canvasClick({0, 0}, 0.01);
    (void)controller.canvasClick({20, 0}, 0.01);
    controller.updatePointer({20, 20}, 0.01);
    if (!controller.overlayState().drawingPolyline()
        || controller.overlayState().drawingPolyline()->vertices.size() != 3) return false;
    (void)controller.canvasClick({20, 20}, 0.01);
    controller.appendCommandCharacter('C');
    if (!controller.confirmInput()) return false;

    if (!controller.invokeCommandText("c")) return false;
    (void)controller.canvasClick({50, 50}, 0.01);
    controller.updatePointer({60, 50}, 0.01);
    if (!controller.overlayState().drawingCircle()
        || !near(controller.overlayState().drawingCircle()->radius, 10.0)) return false;
    if (controller.canvasClick({60, 50}, 0.01) != arz::app::CanvasAction::EntityCreated)
        return false;

    if (!controller.invokeCommandText("a")) return false;
    (void)controller.canvasClick({90, 50}, 0.01);
    (void)controller.canvasClick({80, 60}, 0.01);
    controller.updatePointer({70, 50}, 0.01);
    if (!controller.overlayState().drawingArc()) return false;
    if (controller.canvasClick({70, 50}, 0.01) != arz::app::CanvasAction::EntityCreated)
        return false;

    if (controller.document().objectIds().size() != 3 || !controller.selectAll()
        || !controller.copySelection() || !controller.paste()) return false;
    controller.updatePointer({100, 100}, 0.01);
    const auto& paste = controller.overlayState().pastePlacement();
    if (!paste || paste->polylines.size() != 1 || paste->circles.size() != 1
        || paste->arcs.size() != 1) return false;
    if (controller.canvasClick({100, 100}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectIds().size() != 6) return false;
    if (!controller.undo() || controller.document().objectIds().size() != 3
        || !controller.redo() || controller.document().objectIds().size() != 6) return false;
    return true;
}

bool testTransientPolylineSnapping() {
    arz::app::CadApplicationController controller;
    controller.startPolyline();
    (void)controller.canvasClick({0, 0}, 0.01);
    (void)controller.canvasClick({20, 0}, 0.01);
    (void)controller.canvasClick({20, 20}, 0.01);
    const auto start = controller.snapCandidate({0.2, 0.1}, 0.5);
    const auto midpoint = controller.snapCandidate({10.1, -0.1}, 0.5);
    if (!start || start->objectId != arz::core::InvalidObjectId
        || start->type != arz::cad::SnapType::Endpoint
        || start->point != arz::geometry::Point2D{0, 0}
        || !midpoint || midpoint->objectId != arz::core::InvalidObjectId
        || midpoint->type != arz::cad::SnapType::Midpoint
        || midpoint->point != arz::geometry::Point2D{10, 0}
        || !controller.document().objectIds().empty()) return false;
    (void)controller.toggleDrafting(arz::interaction::DraftingToggle::ObjectSnap);
    if (controller.snapCandidate({0.2, 0.1}, 0.5)) return false;
    (void)controller.toggleDrafting(arz::interaction::DraftingToggle::ObjectSnap);
    if (controller.canvasClick({0, 0}, 0.5) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectIds().size() != 1) return false;
    const auto* polyline = dynamic_cast<const arz::cad::PolylineEntity*>(
        controller.document().object(controller.selectedObjectId()));
    return polyline && polyline->closed() && polyline->vertices().size() == 3;
}

bool testArcThreePointPreviewAndSafeInvalidState() {
    arz::app::CadApplicationController controller;
    controller.startArc();
    (void)controller.canvasClick({0, 0}, 0.01);
    if (!controller.document().objectIds().empty()) return false;
    (void)controller.canvasClick({10, 10}, 0.01);
    if (controller.document().objectIds().size() != 0
        || controller.arcInputState() != arz::interaction::ArcInputState::AwaitingThirdPoint
        || !controller.overlayState().arcReference())
        return false;
    controller.updatePointer({20, 0}, 0.01);
    if (!controller.overlayState().arcReference()
        || !controller.overlayState().drawingArc()) return false;
    const auto firstPreview = *controller.overlayState().drawingArc();
    controller.updatePointer({20, 5}, 0.01);
    if (!controller.overlayState().drawingArc()
        || near(firstPreview.radius, controller.overlayState().drawingArc()->radius)
        || near(firstPreview.sweepAngle, controller.overlayState().drawingArc()->sweepAngle)) return false;
    for (int i = 0; i < 200; ++i)
        controller.updatePointer({20.0, static_cast<double>(i) * 0.05}, 0.01);
    if (!controller.overlayState().drawingArc()
        || controller.document().objectIds().size() != 0) return false;
    const auto preview = *controller.overlayState().drawingArc();
    if (!near(preview.throughPoint.x, 10) || !near(preview.throughPoint.y, 10)
        || !near(preview.endPoint.x, 20) || !near(preview.endPoint.y, 9.95)
        || preview.sweepAngle <= 0.0) return false;
    if (controller.canvasClick({20, 0}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectIds().size() != 1) return false;
    const auto* arc = dynamic_cast<const arz::cad::ArcEntity*>(
        controller.document().object(controller.selectedObjectId()));
    if (!arc || arz::geometry::distanceToArc({10, 10}, arc->center(), arc->radius(),
            arc->startAngle(), arc->endAngle(), arc->counterClockwise()) > 1e-5)
        return false;

    controller.startArc();
    (void)controller.canvasClick({0, 0}, 0.01);
    (void)controller.canvasClick({10, 0}, 0.01);
    controller.updatePointer({20, 0}, 0.01);
    if (controller.overlayState().drawingArc() || !controller.overlayState().arcReference()
        || !controller.document().contains(arc->id()))
        return false;
    (void)controller.escape();
    return controller.arcInputState() == arz::interaction::ArcInputState::Inactive
        && controller.document().objectIds().size() == 1;
}

bool testArcSecondPointReferenceAndSnapping() {
    arz::app::CadApplicationController controller;
    controller.startLine();
    (void)controller.canvasClick({100, 100}, 0.01);
    (void)controller.canvasClick({120, 100}, 0.01);
    const auto baselineObjects = controller.document().objectIds().size();
    controller.startArc();
    (void)controller.canvasClick({0, 0}, 0.01);
    if (controller.arcInputState() != arz::interaction::ArcInputState::AwaitingSecond
        || !controller.overlayState().arcReference()
        || controller.overlayState().arcReference()->stage
            != arz::interaction::ArcReferenceStage::SecondPoint) return false;

    controller.updatePointer({100.3, 100.2}, 1.0);
    auto reference = controller.overlayState().arcReference();
    if (!reference || reference->startPoint != arz::geometry::Point2D{0, 0}
        || reference->secondPoint != arz::geometry::Point2D{100, 100}) return false;
    controller.updatePointer({110, 80}, 0.01);
    reference = controller.overlayState().arcReference();
    if (!reference || reference->secondPoint != arz::geometry::Point2D{110, 80}) return false;

    (void)controller.toggleDrafting(arz::interaction::DraftingToggle::ObjectSnap);
    controller.updatePointer({100.3, 100.2}, 1.0);
    reference = controller.overlayState().arcReference();
    if (!reference || reference->secondPoint != arz::geometry::Point2D{100.3, 100.2}) return false;
    for (int i = 0; i < 300; ++i)
        controller.updatePointer({20.0 + i * 0.1, 30.0 + i * 0.05}, 0.01);
    if (controller.document().objectIds().size() != baselineObjects) return false;

    (void)controller.canvasClick({10, 10}, 0.01);
    if (controller.arcInputState() != arz::interaction::ArcInputState::AwaitingThirdPoint
        || !controller.overlayState().arcReference()
        || controller.overlayState().arcReference()->stage
            != arz::interaction::ArcReferenceStage::ThirdPoint) return false;
    if (!controller.escape()
        || controller.arcInputState() != arz::interaction::ArcInputState::Inactive
        || controller.document().objectIds().size() != baselineObjects) return false;

    arz::app::CadApplicationController emptyController;
    emptyController.startArc();
    (void)emptyController.canvasClick({5, 5}, 0.01);
    emptyController.updatePointer({15, 10}, 0.01);
    return emptyController.escape() && emptyController.document().objectIds().empty();
}

bool testInteractiveCopySelection() {
    arz::app::CadApplicationController controller;
    controller.startLine();
    (void)controller.canvasClick({0, 0}, 0.01);
    (void)controller.canvasClick({10, 0}, 0.01);
    const auto sourceId = controller.selectedObjectId();
    if (!sourceId || controller.document().objectIds().size() != 1 || !controller.selectAll()) return false;
    if (!controller.copySelection() || controller.copyInputState() != arz::interaction::CopyInputState::Inactive) return false;
    if (!controller.startCopySelection() || controller.copyInputState() != arz::interaction::CopyInputState::AwaitingBasePoint) return false;
    (void)controller.canvasClick({0, 0}, 0.5);
    if (controller.copyInputState() != arz::interaction::CopyInputState::AwaitingDestination
        || controller.document().objectIds().size() != 1) return false;
    const auto fixedBase = controller.copyBasePoint();
    for (int i = 0; i < 100; ++i) controller.updatePointer({20.0 + i * 0.1, 5.0}, 0.01);
    if (!controller.overlayState().pastePlacement() || controller.document().objectIds().size() != 1) return false;
    if (controller.canvasClick({20, 5}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectIds().size() != 2
        || controller.copyInputState() != arz::interaction::CopyInputState::AwaitingDestination
        || controller.copyBasePoint() != fixedBase) return false;
    controller.updatePointer({40, 10}, 0.01);
    if (controller.canvasClick({40, 10}, 0.01) != arz::app::CanvasAction::EntityCreated) return false;
    controller.updatePointer({-10, 20}, 0.01);
    if (controller.canvasClick({-10, 20}, 0.01) != arz::app::CanvasAction::EntityCreated
        || controller.document().objectIds().size() != 4
        || controller.copyBasePoint() != fixedBase) return false;
    const auto ids = controller.document().objectIds();
    if (ids.size() != 4 || !controller.document().contains(sourceId)) return false;
    const auto* copied = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[1]));
    const auto* copied2 = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[2]));
    const auto* copied3 = dynamic_cast<const arz::cad::LineEntity*>(controller.document().object(ids[3]));
    if (!copied || !near(copied->start().x, 20) || !near(copied->start().y, 5)
        || !near(copied->end().x, 30) || !near(copied->end().y, 5)
        || !copied2 || !near(copied2->start().x, 40) || !near(copied2->end().x, 50)
        || !copied3 || !near(copied3->start().x, -10) || !near(copied3->end().x, 0)) return false;
    if (!controller.undo() || controller.document().objectIds().size() != 3
        || controller.copyInputState() != arz::interaction::CopyInputState::AwaitingDestination
        || !controller.undo() || controller.document().objectIds().size() != 2
        || !controller.redo() || controller.document().objectIds().size() != 3) return false;
    if (!controller.confirmInput() || controller.copyInputState() != arz::interaction::CopyInputState::Inactive) return false;
    (void)controller.selectAll();
    if (!controller.startCopySelection() || !controller.escape()) return false;
    (void)controller.selectAll();
    if (!controller.startCopySelection() || !controller.rightClick()) return false;
    return controller.copyInputState() == arz::interaction::CopyInputState::Inactive
        && controller.document().objectIds().size() == 3;
}

bool testAliasesParserAndRenderAdapters() {
    arz::interaction::CommandRegistry registry;
    if (!registry.resolve("PL") || registry.resolve("PL")->canonicalName != "PLINE"
        || !registry.resolve("C") || registry.resolve("C")->canonicalName != "CIRCLE"
        || !registry.resolve("A") || registry.resolve("A")->canonicalName != "ARC"
        || !registry.resolve("CO") || registry.resolve("CO")->canonicalName != "COPY"
        || arz::interaction::CommandLineParser::parse("pline")
            != arz::interaction::EditorCommand::Polyline
        || arz::interaction::CommandLineParser::parse("c")
            != arz::interaction::EditorCommand::Circle
        || arz::interaction::CommandLineParser::parse("a")
            != arz::interaction::EditorCommand::Arc
        || arz::interaction::CommandLineParser::parse("co")
            != arz::interaction::EditorCommand::Copy) return false;

    arz::core::Document document;
    if (!document.addObject(std::make_unique<arz::cad::PolylineEntity>(
            30, arz::cad::DefaultLayerId,
            std::vector<arz::geometry::Point2D>{{0, 0}, {10, 0}}, false))
        || !document.addObject(std::make_unique<arz::cad::CircleEntity>(
            10, arz::cad::DefaultLayerId, arz::geometry::Point2D{20, 20}, 5))
        || !document.addObject(std::make_unique<arz::cad::ArcEntity>(
            20, arz::cad::DefaultLayerId, arz::geometry::Point2D{40, 40}, 5,
            0, std::numbers::pi, true))) return false;
    arz::rendering::Viewport2D viewport(1.0);
    viewport.setScreenOrigin({100, 100});
    const std::vector<arz::core::ObjectId> selected{20};
    const auto context = arz::rendering::makeRenderContext(
        viewport, {200, 200}, selected);
    arz::rendering::RenderScene scene;
    arz::rendering::DocumentRenderPlanner planner;
    planner.build(document, context, scene);
    if (scene.primitives().size() != 3
        || !std::holds_alternative<arz::rendering::CircleRenderPrimitive>(scene.primitives()[0])
        || !std::holds_alternative<arz::rendering::ArcRenderPrimitive>(scene.primitives()[1])
        || !std::holds_alternative<arz::rendering::PolylineRenderPrimitive>(scene.primitives()[2]))
        return false;
    return std::get<arz::rendering::ArcRenderPrimitive>(scene.primitives()[1]).style.selected;
}

}

int main() {
    int failures = 0;
    const auto run = [&failures](std::string_view name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
        if (!passed) ++failures;
    };
    run("EntitiesAndUndoRedoCommands", testEntitiesAndUndoRedoCommands());
    run("SelectionAndSnappingCoverage", testSelectionAndSnappingCoverage());
    run("InteractiveCommandsPreviewsAndClipboard", testInteractiveCommandsPreviewsAndClipboard());
    run("TransientPolylineSnapping", testTransientPolylineSnapping());
    run("ArcThreePointPreviewAndSafeInvalidState", testArcThreePointPreviewAndSafeInvalidState());
    run("ArcSecondPointReferenceAndSnapping", testArcSecondPointReferenceAndSnapping());
    run("InteractiveCopySelection", testInteractiveCopySelection());
    run("AliasesParserAndRenderAdapters", testAliasesParserAndRenderAdapters());
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
