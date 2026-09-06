#include "cad/entities/CadEntity.h"

namespace arz::cad {

CadEntity::CadEntity(
    arz::core::ObjectId id,
    LayerId layerId
)
    : DocumentObject(id),
      layerId_(layerId) {
}

LayerId CadEntity::layerId() const noexcept {
    return layerId_;
}

void CadEntity::setLayerId(
    LayerId layerId
) noexcept {
    layerId_ = layerId;
}

EntityGraphicsProperties&
CadEntity::graphics() noexcept {
    return graphics_;
}

const EntityGraphicsProperties&
CadEntity::graphics() const noexcept {
    return graphics_;
}

}
