#pragma once

#include "geometry/primitives/Point2D.h"

namespace arz::cad {

class CadEntity;

class IEntityPickRefiner {
public:
    virtual ~IEntityPickRefiner() = default;

    [[nodiscard]] virtual bool hit(
        const CadEntity& entity,
        arz::geometry::Point2D point,
        double tolerance
    ) const noexcept = 0;
};

}
