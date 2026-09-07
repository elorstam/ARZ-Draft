#pragma once

#include <vector>

#include "core/objects/ObjectId.h"
#include "geometry/primitives/Point2D.h"

namespace arz::core {
class Document;
}

namespace arz::cad {

class IEntityPickRefiner;
class ISpatialIndex2D;

class SelectionService final {
public:
    SelectionService(
        const arz::core::Document& document,
        const ISpatialIndex2D& spatialIndex,
        const IEntityPickRefiner& pickRefiner
    );

    [[nodiscard]] std::vector<arz::core::ObjectId> pointPick(
        arz::geometry::Point2D point,
        double tolerance
    ) const;

    [[nodiscard]] std::vector<arz::core::ObjectId>
    crossingWindow(
        arz::geometry::Point2D first,
        arz::geometry::Point2D second
    ) const;

    [[nodiscard]] std::vector<arz::core::ObjectId>
    containedWindow(
        arz::geometry::Point2D first,
        arz::geometry::Point2D second
    ) const;

private:
    const arz::core::Document& document_;
    const ISpatialIndex2D& spatialIndex_;
    const IEntityPickRefiner& pickRefiner_;
};

}
