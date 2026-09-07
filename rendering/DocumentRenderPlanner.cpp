#include "rendering/DocumentRenderPlanner.h"

#include <algorithm>
#include <array>

#include "cad/entities/CadEntity.h"
#include "core/document/Document.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"

namespace arz::rendering {

namespace {

bool isVisible(const arz::cad::CadEntity& entity,
               const arz::core::Document& document) {
    const auto* layer = document.layers().get(entity.layerId());
    return layer != nullptr && layer->visible() && !layer->frozen();
}

}

void DocumentRenderPlanner::build(
    const arz::core::Document& document,
    const RenderContext& context,
    RenderScene& scene
) const {
    auto objectIds = document.objectIds();
    std::ranges::sort(objectIds);
    scene.prepare(objectIds.size());
    const std::array<const IEntityRenderAdapter*, 4> adapters{
        &lineAdapter_, &polylineAdapter_, &circleAdapter_, &arcAdapter_};
    for (const auto objectId : objectIds) {
        const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(
            document.object(objectId));
        if (entity == nullptr || !isVisible(*entity, document)
            || !entity->boundingBox().intersects(context.visibleWorldBounds)) {
            continue;
        }
        for (const auto* adapter : adapters) {
            if (adapter->supports(entity->entityType())) {
                (void)adapter->append(*entity, document, context, scene);
                break;
            }
        }
    }
}

std::optional<arz::geometry::BoundingBox2D>
DocumentRenderPlanner::visibleDocumentBounds(
    const arz::core::Document& document
) const {
    std::optional<arz::geometry::BoundingBox2D> result;
    for (const auto objectId : document.objectIds()) {
        const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(
            document.object(objectId));
        if (entity == nullptr || !isVisible(*entity, document)) continue;
        const auto bounds = entity->boundingBox().normalized();
        if (!result) result = bounds;
        else {
            result->minX = std::min(result->minX, bounds.minX);
            result->minY = std::min(result->minY, bounds.minY);
            result->maxX = std::max(result->maxX, bounds.maxX);
            result->maxY = std::max(result->maxY, bounds.maxY);
        }
    }
    return result;
}

}
