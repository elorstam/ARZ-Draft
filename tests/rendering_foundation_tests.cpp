#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"
#include "rendering/DocumentRenderPlanner.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"
#include "rendering/ScreenLineWeightPolicy.h"
#include "rendering/Viewport2D.h"

namespace {

bool near(double first, double second, double tolerance = 1e-9) {
    return std::abs(first - second) <= tolerance;
}

bool addLine(arz::core::Document& document,
             arz::core::ObjectId id,
             arz::geometry::Point2D start,
             arz::geometry::Point2D end) {
    return document.addObject(std::make_unique<arz::cad::LineEntity>(
        id, arz::cad::DefaultLayerId, start, end));
}

bool testLineWeightScreenMapping() {
    using arz::cad::LineWeight;
    constexpr std::array weights{
        LineWeight::W005, LineWeight::W009, LineWeight::W013,
        LineWeight::W018, LineWeight::W025, LineWeight::W035,
        LineWeight::W050, LineWeight::W070, LineWeight::W100
    };
    double previous = 0.0;
    for (const auto weight : weights) {
        const double width = arz::rendering::ScreenLineWeightPolicy::pixelWidth(weight);
        if (width < 1.0 || width < previous) return false;
        previous = width;
    }
    return near(arz::rendering::ScreenLineWeightPolicy::pixelWidth(LineWeight::W005), 1.0)
        && near(arz::rendering::ScreenLineWeightPolicy::pixelWidth(LineWeight::W025), 1.0)
        && near(arz::rendering::ScreenLineWeightPolicy::pixelWidth(LineWeight::W035),
                0.35 * 96.0 / 25.4)
        && near(arz::rendering::ScreenLineWeightPolicy::pixelWidth(LineWeight::W100),
                96.0 / 25.4);
}

bool testViewportVisibleBoundsAndRoundTrip() {
    arz::rendering::Viewport2D viewport(0.2);
    viewport.setScreenOrigin({400.0, 300.0});
    const auto bounds = viewport.visibleWorldBounds({800.0, 600.0});
    if (!near(bounds.minX, -2000.0) || !near(bounds.maxX, 2000.0)
        || !near(bounds.minY, -1500.0) || !near(bounds.maxY, 1500.0)) return false;
    for (const auto point : std::array{
            arz::geometry::Point2D{-1234.5, 678.25},
            arz::geometry::Point2D{0.0, 0.0},
            arz::geometry::Point2D{9876.5, -4321.25}}) {
        const auto roundTrip = viewport.screenToWorld(viewport.worldToScreen(point));
        if (!near(roundTrip.x, point.x) || !near(roundTrip.y, point.y)) return false;
    }
    const arz::geometry::Point2D cursor{175.0, 220.0};
    const auto anchored = viewport.screenToWorld(cursor);
    viewport.zoomAt(cursor, 1.75);
    const auto afterZoom = viewport.screenToWorld(cursor);
    if (!near(anchored.x, afterZoom.x) || !near(anchored.y, afterZoom.y)) return false;
    const auto beforePan = viewport.screenOrigin();
    viewport.pan({15.0, -25.0});
    return viewport.screenOrigin()
        == arz::geometry::Point2D{beforePan.x + 15.0, beforePan.y - 25.0};
}

bool testCullingOrderingResolutionAndSelectionIsolation() {
    arz::core::Document document;
    auto* layer = document.layers().get(arz::cad::DefaultLayerId);
    if (!layer) return false;
    layer->setColor(arz::cad::Color::redColor());
    layer->setLineWeight(arz::cad::LineWeight::W035);
    if (!addLine(document, 30, {100.0, 100.0}, {200.0, 200.0})
        || !addLine(document, 20, {-60.0, 0.0}, {0.0, 0.0})
        || !addLine(document, 10, {-10.0, -10.0}, {10.0, 10.0})) return false;
    auto* selectedLine = dynamic_cast<arz::cad::LineEntity*>(document.object(10));
    if (!selectedLine) return false;
    selectedLine->graphics().colorSource = arz::cad::GraphicsPropertySource::Explicit;
    selectedLine->graphics().color = arz::cad::Color::cyanColor();
    selectedLine->graphics().lineWeightSource = arz::cad::GraphicsPropertySource::Explicit;
    selectedLine->graphics().lineWeight = arz::cad::LineWeight::W070;
    const auto graphicsBefore = selectedLine->graphics();

    arz::rendering::Viewport2D viewport(1.0);
    viewport.setScreenOrigin({50.0, 50.0});
    const std::vector<arz::core::ObjectId> selection{10};
    const auto context = arz::rendering::makeRenderContext(
        viewport, {100.0, 100.0}, selection);
    arz::rendering::DocumentRenderPlanner planner;
    arz::rendering::RenderScene scene;
    planner.build(document, context, scene);

    if (scene.primitives().size() != 2) return false;
    const auto& first = std::get<arz::rendering::LineRenderPrimitive>(scene.primitives()[0]);
    const auto& second = std::get<arz::rendering::LineRenderPrimitive>(scene.primitives()[1]);
    return first.objectId == 10 && second.objectId == 20
        && first.style.color == arz::cad::Color::cyanColor()
        && first.style.lineWeight == arz::cad::LineWeight::W070
        && first.style.selected && !second.style.selected
        && second.style.color == arz::cad::Color::redColor()
        && second.style.lineWeight == arz::cad::LineWeight::W035
        && selectedLine->graphics() == graphicsBefore;
}

}

int main() {
    int failures = 0;
    const auto run = [&failures](std::string_view name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name << '\n';
        if (!passed) ++failures;
    };
    run("LineWeightScreenMapping", testLineWeightScreenMapping());
    run("ViewportVisibleBoundsAndRoundTrip", testViewportVisibleBoundsAndRoundTrip());
    run("CullingOrderingResolutionAndSelectionIsolation",
        testCullingOrderingResolutionAndSelectionIsolation());
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
