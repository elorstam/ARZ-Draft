#pragma once

#include "geometry/primitives/Point2D.h"
#include "geometry/primitives/BoundingBox2D.h"

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

    [[nodiscard]] virtual bool intersects(
        const CadEntity& entity,
        arz::geometry::BoundingBox2D window
    ) const noexcept = 0;

    [[nodiscard]] virtual bool contained(
        const CadEntity& entity,
        arz::geometry::BoundingBox2D window
    ) const noexcept = 0;
};

}
