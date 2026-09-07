#pragma once

#include "cad/selection/IEntityPickRefiner.h"

namespace arz::cad {

class LineEntityPickRefiner final : public IEntityPickRefiner {
public:
    [[nodiscard]] bool hit(
        const CadEntity& entity,
        arz::geometry::Point2D point,
        double tolerance
    ) const noexcept override;
};

}
