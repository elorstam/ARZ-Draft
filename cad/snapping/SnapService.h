#pragma once

#include <optional>
#include <span>
#include <vector>

#include "cad/snapping/SnapResult.h"

namespace arz::core {
class Document;
}

namespace arz::cad {

class IEntitySnapProvider;
class ISpatialIndex2D;

class SnapService final {
public:
    SnapService(
        const arz::core::Document& document,
        const ISpatialIndex2D& spatialIndex,
        const IEntitySnapProvider& snapProvider
    );

    [[nodiscard]] std::vector<SnapResult> candidates(
        arz::geometry::Point2D queryPoint,
        double tolerance,
        std::span<const SnapType> enabledTypes
    ) const;

    [[nodiscard]] std::optional<SnapResult> bestSnap(
        arz::geometry::Point2D queryPoint,
        double tolerance,
        std::span<const SnapType> enabledTypes
    ) const;

private:
    const arz::core::Document& document_;
    const ISpatialIndex2D& spatialIndex_;
    const IEntitySnapProvider& snapProvider_;
};

}
