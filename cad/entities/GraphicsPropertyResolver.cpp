#include "cad/entities/GraphicsPropertyResolver.h"

namespace arz::cad {

ResolvedGraphicsProperties
GraphicsPropertyResolver::resolve(
    const EntityGraphicsProperties& entity,
    const Layer& layer
) noexcept {
    ResolvedGraphicsProperties result{
        layer.color(),
        layer.lineTypeId(),
        layer.lineWeight()
    };

    if (entity.colorSource ==
        GraphicsPropertySource::Explicit) {
        result.color = entity.color;
    }

    if (entity.lineTypeSource ==
        GraphicsPropertySource::Explicit) {
        result.lineTypeId = entity.lineTypeId;
    }

    if (entity.lineWeightSource ==
        GraphicsPropertySource::Explicit) {
        result.lineWeight = entity.lineWeight;
    }

    return result;
}

}
