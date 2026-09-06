#include <cstdlib>
#include <iostream>

#include "cad/entities/GraphicsPropertyResolver.h"
#include "cad/entities/LineEntity.h"
#include "cad/layers/Layer.h"

namespace {

bool testByLayerResolution() {
    arz::cad::Layer layer(2, "A-WALL");

    layer.setColor(
        arz::cad::Color::redColor()
    );

    layer.setLineWeight(
        arz::cad::LineWeight::W050
    );

    arz::cad::LineEntity line(
        1,
        layer.id(),
        {0.0, 0.0},
        {1000.0, 0.0}
    );

    const auto resolved =
        arz::cad::GraphicsPropertyResolver::resolve(
            line.graphics(),
            layer
        );

    return resolved.color
            == arz::cad::Color::redColor()
        && resolved.lineWeight
            == arz::cad::LineWeight::W050;
}

bool testExplicitColorOverride() {
    arz::cad::Layer layer(2, "A-WALL");

    layer.setColor(
        arz::cad::Color::redColor()
    );

    arz::cad::LineEntity line(
        1,
        layer.id(),
        {0.0, 0.0},
        {1000.0, 0.0}
    );

    line.graphics().colorSource =
        arz::cad::GraphicsPropertySource::Explicit;

    line.graphics().color =
        arz::cad::Color::cyanColor();

    const auto resolved =
        arz::cad::GraphicsPropertyResolver::resolve(
            line.graphics(),
            layer
        );

    return resolved.color
        == arz::cad::Color::cyanColor();
}

bool testExplicitLineWeightOverride() {
    arz::cad::Layer layer(2, "A-DIMS");

    layer.setLineWeight(
        arz::cad::LineWeight::W018
    );

    arz::cad::LineEntity line(
        1,
        layer.id(),
        {0.0, 0.0},
        {1000.0, 0.0}
    );

    line.graphics().lineWeightSource =
        arz::cad::GraphicsPropertySource::Explicit;

    line.graphics().lineWeight =
        arz::cad::LineWeight::W070;

    const auto resolved =
        arz::cad::GraphicsPropertyResolver::resolve(
            line.graphics(),
            layer
        );

    return resolved.lineWeight
        == arz::cad::LineWeight::W070;
}

bool testDefaultEntityUsesByLayer() {
    arz::cad::LineEntity line(
        1,
        arz::cad::DefaultLayerId,
        {0.0, 0.0},
        {1000.0, 0.0}
    );

    return line.graphics().colorSource
            == arz::cad::GraphicsPropertySource::ByLayer
        && line.graphics().lineTypeSource
            == arz::cad::GraphicsPropertySource::ByLayer
        && line.graphics().lineWeightSource
            == arz::cad::GraphicsPropertySource::ByLayer;
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

    run(
        "ByLayerResolution",
        testByLayerResolution()
    );

    run(
        "ExplicitColorOverride",
        testExplicitColorOverride()
    );

    run(
        "ExplicitLineWeightOverride",
        testExplicitLineWeightOverride()
    );

    run(
        "DefaultEntityUsesByLayer",
        testDefaultEntityUsesByLayer()
    );

    if (failures != 0) {
        return EXIT_FAILURE;
    }

    std::cout
        << "All graphics tests passed.\n";

    return EXIT_SUCCESS;
}
