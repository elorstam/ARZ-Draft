#pragma once

#include "cad/snapping/SnapType.h"
#include "core/objects/ObjectId.h"
#include "geometry/primitives/Point2D.h"

namespace arz::cad {

struct SnapPoint final {
    SnapType type{SnapType::Endpoint};
    arz::geometry::Point2D point{};
};

struct SnapResult final {
    arz::core::ObjectId objectId{arz::core::InvalidObjectId};
    SnapType type{SnapType::Endpoint};
    arz::geometry::Point2D point{};
    double distance{0.0};
};

}
