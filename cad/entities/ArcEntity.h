#pragma once

#include "cad/entities/CadEntity.h"
#include "geometry/primitives/Point2D.h"

namespace arz::cad {

class ArcEntity final : public CadEntity {
public:
    ArcEntity(arz::core::ObjectId id, LayerId layerId,
              arz::geometry::Point2D center, double radius,
              double startAngle, double endAngle, bool counterClockwise);
    [[nodiscard]] EntityType entityType() const noexcept override;
    [[nodiscard]] arz::geometry::Point2D center() const noexcept;
    [[nodiscard]] double radius() const noexcept;
    [[nodiscard]] double startAngle() const noexcept;
    [[nodiscard]] double endAngle() const noexcept;
    [[nodiscard]] bool counterClockwise() const noexcept;
    [[nodiscard]] double sweepAngle() const noexcept;
    [[nodiscard]] double length() const noexcept;
    [[nodiscard]] arz::geometry::Point2D startPoint() const noexcept;
    [[nodiscard]] arz::geometry::Point2D endPoint() const noexcept;
    [[nodiscard]] arz::geometry::BoundingBox2D boundingBox() const noexcept override;

private:
    arz::geometry::Point2D center_{};
    double radius_{};
    double startAngle_{};
    double endAngle_{};
    bool counterClockwise_{true};
};

}
