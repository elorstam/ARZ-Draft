#pragma once

#include "cad/selection/IEntityPickRefiner.h"

namespace arz::cad {

class CadEntityPickRefiner final : public IEntityPickRefiner {
public:
    [[nodiscard]] bool hit(const CadEntity& entity,
        arz::geometry::Point2D point, double tolerance) const noexcept override;
    [[nodiscard]] bool intersects(const CadEntity& entity,
        arz::geometry::BoundingBox2D window) const noexcept override;
    [[nodiscard]] bool contained(const CadEntity& entity,
        arz::geometry::BoundingBox2D window) const noexcept override;
};

}
