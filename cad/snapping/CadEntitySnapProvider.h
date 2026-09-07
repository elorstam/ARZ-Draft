#pragma once

#include "cad/snapping/IEntitySnapProvider.h"

namespace arz::cad {

class CadEntitySnapProvider final : public IEntitySnapProvider {
public:
    void appendSnapPoints(const CadEntity& entity,
        std::span<const SnapType> enabledTypes,
        std::vector<SnapPoint>& points) const override;
};

}
