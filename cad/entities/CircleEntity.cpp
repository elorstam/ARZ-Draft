#include "cad/entities/CircleEntity.h"

#include <cmath>
#include <numbers>
#include <stdexcept>

#include "geometry/algorithms/Curve2D.h"

namespace arz::cad {

CircleEntity::CircleEntity(arz::core::ObjectId id, LayerId layerId,
                           arz::geometry::Point2D center, double radius)
    : CadEntity(id, layerId), center_(center), radius_(radius) {
    if (!std::isfinite(radius_) || radius_ < 0.0)
        throw std::invalid_argument("Circle radius must be finite and non-negative");
}
EntityType CircleEntity::entityType() const noexcept { return EntityType::Circle; }
arz::geometry::Point2D CircleEntity::center() const noexcept { return center_; }
double CircleEntity::radius() const noexcept { return radius_; }
double CircleEntity::diameter() const noexcept { return radius_ * 2.0; }
double CircleEntity::circumference() const noexcept { return radius_ * arz::geometry::TwoPi; }
double CircleEntity::area() const noexcept { return std::numbers::pi * radius_ * radius_; }
arz::geometry::BoundingBox2D CircleEntity::boundingBox() const noexcept {
    return arz::geometry::circleBounds(center_, radius_);
}

}
