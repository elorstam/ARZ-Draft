#pragma once

#include <cstddef>
#include <vector>

#include "core/objects/ObjectId.h"
#include "geometry/primitives/BoundingBox2D.h"

namespace arz::cad {

class ISpatialIndex2D {
public:
    virtual ~ISpatialIndex2D() = default;

    virtual bool insert(
        arz::core::ObjectId objectId,
        arz::geometry::BoundingBox2D bounds
    ) = 0;

    virtual bool update(
        arz::core::ObjectId objectId,
        arz::geometry::BoundingBox2D bounds
    ) = 0;

    virtual bool remove(arz::core::ObjectId objectId) = 0;
    virtual void clear() noexcept = 0;

    [[nodiscard]] virtual std::vector<arz::core::ObjectId>
    query(const arz::geometry::BoundingBox2D& bounds) const = 0;

    [[nodiscard]] virtual std::size_t size() const noexcept = 0;
};

}
