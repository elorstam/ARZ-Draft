#include "rendering/ArcRenderAdapter.h"
#include "cad/entities/ArcEntity.h"
#include "cad/entities/GraphicsPropertyResolver.h"
#include "core/document/Document.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"
namespace arz::rendering {
bool ArcRenderAdapter::supports(arz::cad::EntityType type) const noexcept { return type == arz::cad::EntityType::Arc; }
bool ArcRenderAdapter::append(const arz::cad::CadEntity& entity,
    const arz::core::Document& document, const RenderContext& context, RenderScene& scene) const {
    const auto* value = dynamic_cast<const arz::cad::ArcEntity*>(&entity);
    const auto* layer = document.layers().get(entity.layerId());
    if (!value || !layer) return false;
    const auto resolved = arz::cad::GraphicsPropertyResolver::resolve(value->graphics(), *layer);
    scene.append({value->id(), value->center(), value->radius(), value->startAngle(),
        value->sweepAngle(), value->counterClockwise(),
        {resolved.color, resolved.lineWeight, context.isSelected(value->id())}});
    return true;
}
}
