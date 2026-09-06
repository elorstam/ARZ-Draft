#pragma once

#include "cad/entities/EntityGraphicsProperties.h"
#include "cad/layers/Layer.h"

namespace arz::cad {

class GraphicsPropertyResolver final {
public:
    [[nodiscard]] static ResolvedGraphicsProperties resolve(
        const EntityGraphicsProperties& entity,
        const Layer& layer
    ) noexcept;
};

}
