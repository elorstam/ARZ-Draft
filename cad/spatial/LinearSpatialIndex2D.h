#pragma once

#include <map>

#include "cad/spatial/ISpatialIndex2D.h"

namespace arz::cad {

class LinearSpatialIndex2D final : public ISpatialIndex2D {
public:
    bool insert(
        arz::core::ObjectId objectId,
        arz::geometry::BoundingBox2D bounds
    ) override;

    bool update(
        arz::core::ObjectId objectId,
        arz::geometry::BoundingBox2D bounds
    ) override;

    bool remove(arz::core::ObjectId objectId) override;
    void clear() noexcept override;

    [[nodiscard]] std::vector<arz::core::ObjectId>
    query(
        const arz::geometry::BoundingBox2D& bounds
    ) const override;

    [[nodiscard]] std::size_t size() const noexcept override;

private:
    std::map<
        arz::core::ObjectId,
        arz::geometry::BoundingBox2D
    > entries_;
};

}
