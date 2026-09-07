#pragma once

#include "cad/entities/CadEntity.h"
#include "geometry/primitives/Point2D.h"

namespace arz::cad {

class CircleEntity final : public CadEntity {
public:
    CircleEntity(arz::core::ObjectId id, LayerId layerId,
                 arz::geometry::Point2D center, double radius);
    [[nodiscard]] EntityType entityType() const noexcept override;
    [[nodiscard]] arz::geometry::Point2D center() const noexcept;
    [[nodiscard]] double radius() const noexcept;
    [[nodiscard]] double diameter() const noexcept;
    [[nodiscard]] double circumference() const noexcept;
    [[nodiscard]] double area() const noexcept;
    [[nodiscard]] arz::geometry::BoundingBox2D boundingBox() const noexcept override;

private:
    arz::geometry::Point2D center_{};
    double radius_{};
};

}
