#pragma once

#include <span>
#include <vector>

#include "cad/snapping/SnapResult.h"

namespace arz::cad {

class CadEntity;

class IEntitySnapProvider {
public:
    virtual ~IEntitySnapProvider() = default;

    virtual void appendSnapPoints(
        const CadEntity& entity,
        std::span<const SnapType> enabledTypes,
        std::vector<SnapPoint>& points
    ) const = 0;
};

}
