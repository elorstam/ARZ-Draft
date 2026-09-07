#include "cad/spatial/SpatialIndexSynchronizer.h"

#include "cad/entities/CadEntity.h"
#include "cad/spatial/ISpatialIndex2D.h"
#include "core/document/Document.h"

namespace arz::cad {

bool SpatialIndexSynchronizer::rebuild(
    const arz::core::Document& document,
    ISpatialIndex2D& spatialIndex
) {
    spatialIndex.clear();

    for (const auto objectId : document.objectIds()) {
        const auto* entity = dynamic_cast<const CadEntity*>(
            document.object(objectId)
        );

        if (entity != nullptr
            && !spatialIndex.insert(
                objectId,
                entity->boundingBox()
            )) {
            spatialIndex.clear();
            return false;
        }
    }

    return true;
}

}
