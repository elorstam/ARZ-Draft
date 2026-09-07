#pragma once

#include <vector>

#include "cad/entities/CadEntity.h"
#include "geometry/primitives/Point2D.h"

namespace arz::cad {

class PolylineEntity final : public CadEntity {
public:
    PolylineEntity(arz::core::ObjectId id, LayerId layerId,
                   std::vector<arz::geometry::Point2D> vertices,
                   bool closed = false);
    [[nodiscard]] EntityType entityType() const noexcept override;
    [[nodiscard]] const std::vector<arz::geometry::Point2D>& vertices() const noexcept;
    [[nodiscard]] bool closed() const noexcept;
    [[nodiscard]] double length() const noexcept;
    [[nodiscard]] arz::geometry::BoundingBox2D boundingBox() const noexcept override;

private:
    std::vector<arz::geometry::Point2D> vertices_;
    bool closed_{};
};

}
