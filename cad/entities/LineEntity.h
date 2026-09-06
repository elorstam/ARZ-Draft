#pragma once

#include "cad/entities/CadEntity.h"
#include "geometry/primitives/Point2D.h"

namespace arz::cad {

class LineEntity final : public CadEntity {
public:
    LineEntity(
        arz::core::ObjectId id,
        LayerId layerId,
        arz::geometry::Point2D start,
        arz::geometry::Point2D end
    );

    [[nodiscard]] EntityType entityType() const noexcept override;

    [[nodiscard]] arz::geometry::Point2D start() const noexcept;
    [[nodiscard]] arz::geometry::Point2D end() const noexcept;

    void setStart(arz::geometry::Point2D point) noexcept;
    void setEnd(arz::geometry::Point2D point) noexcept;

    [[nodiscard]] double length() const noexcept;

    [[nodiscard]] arz::geometry::BoundingBox2D
    boundingBox() const noexcept override;

private:
    arz::geometry::Point2D start_;
    arz::geometry::Point2D end_;
};

}
