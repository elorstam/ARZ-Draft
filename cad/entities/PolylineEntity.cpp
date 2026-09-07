#include "cad/entities/PolylineEntity.h"

#include <stdexcept>
#include <utility>

#include "geometry/algorithms/Curve2D.h"

namespace arz::cad {

PolylineEntity::PolylineEntity(arz::core::ObjectId id, LayerId layerId,
                               std::vector<arz::geometry::Point2D> vertices,
                               bool closed)
    : CadEntity(id, layerId), vertices_(std::move(vertices)), closed_(closed) {
    if (vertices_.size() < 2 || (closed_ && vertices_.size() < 3))
        throw std::invalid_argument("Polyline requires at least two vertices; closed requires three");
}
EntityType PolylineEntity::entityType() const noexcept { return EntityType::Polyline; }
const std::vector<arz::geometry::Point2D>& PolylineEntity::vertices() const noexcept { return vertices_; }
bool PolylineEntity::closed() const noexcept { return closed_; }
double PolylineEntity::length() const noexcept {
    return arz::geometry::polylineLength(vertices_, closed_);
}
arz::geometry::BoundingBox2D PolylineEntity::boundingBox() const noexcept {
    return arz::geometry::polylineBounds(vertices_);
}

}
