#include <cstdlib>
#include <iostream>

#include "cad/layers/Color.h"
#include "cad/layers/Layer.h"
#include "cad/layers/LayerTable.h"
#include "cad/layers/LineWeight.h"
#include "core/document/Document.h"

namespace {

bool testDefaultLayer() {
    arz::cad::LayerTable layers;

    if (layers.size() != 1) {
        return false;
    }

    const auto* layer =
        layers.get(arz::cad::DefaultLayerId);

    return layer != nullptr
        && layer->name() == "0"
        && layer->visible()
        && !layer->frozen()
        && !layer->locked()
        && layer->plottable();
}

bool testCreateLayer() {
    arz::cad::LayerTable layers;

    const auto id = layers.nextLayerId();

    arz::cad::Layer layer(
        id,
        "A-WALL"
    );

    layer.setColor(
        arz::cad::Color::redColor()
    );

    layer.setLineWeight(
        arz::cad::LineWeight::W035
    );

    if (!layers.add(std::move(layer))) {
        return false;
    }

    const auto* stored = layers.get(id);

    return stored != nullptr
        && stored->name() == "A-WALL"
        && stored->color()
            == arz::cad::Color::redColor()
        && stored->lineWeight()
            == arz::cad::LineWeight::W035;
}

bool testDuplicateNameRejected() {
    arz::cad::LayerTable layers;

    const auto first = layers.nextLayerId();
    const auto second = layers.nextLayerId();

    if (!layers.add(
            arz::cad::Layer(first, "A-WALL"))) {
        return false;
    }

    return !layers.add(
        arz::cad::Layer(second, "A-WALL")
    );
}

bool testDefaultLayerCannotBeRemoved() {
    arz::cad::LayerTable layers;

    return !layers.remove(
        arz::cad::DefaultLayerId
    );
}

bool testLayerFlags() {
    arz::cad::Layer layer(
        25,
        "A-DIMS"
    );

    layer.setVisible(false);
    layer.setFrozen(true);
    layer.setLocked(true);
    layer.setPlottable(false);

    return !layer.visible()
        && layer.frozen()
        && layer.locked()
        && !layer.plottable();
}

bool testLineWeight() {
    return arz::cad::lineWeightMillimeters(
        arz::cad::LineWeight::W035
    ) == 0.35;
}

bool testDocumentLayerTable() {
    arz::core::Document document;

    auto& layers = document.layers();

    const auto id = layers.nextLayerId();

    if (!layers.add(
            arz::cad::Layer(
                id,
                "A-DOOR"
            ))) {
        return false;
    }

    return document.layers().contains(id)
        && document.layers()
            .containsName("A-DOOR");
}

}

int main() {
    int failures = 0;

    const auto run = [&failures](
        const char* name,
        bool result
    ) {
        if (result) {
            std::cout
                << "[PASS] "
                << name
                << '\n';
        } else {
            std::cerr
                << "[FAIL] "
                << name
                << '\n';

            ++failures;
        }
    };

    run("DefaultLayer", testDefaultLayer());
    run("CreateLayer", testCreateLayer());
    run(
        "DuplicateNameRejected",
        testDuplicateNameRejected()
    );
    run(
        "DefaultLayerCannotBeRemoved",
        testDefaultLayerCannotBeRemoved()
    );
    run("LayerFlags", testLayerFlags());
    run("LineWeight", testLineWeight());
    run(
        "DocumentLayerTable",
        testDocumentLayerTable()
    );

    if (failures != 0) {
        std::cerr
            << failures
            << " layer test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All layer tests passed.\n";

    return EXIT_SUCCESS;
}
