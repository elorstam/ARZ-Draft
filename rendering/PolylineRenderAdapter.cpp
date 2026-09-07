#include "rendering/PolylineRenderAdapter.h"
#include "cad/entities/GraphicsPropertyResolver.h"
#include "cad/entities/PolylineEntity.h"
#include "core/document/Document.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"
namespace arz::rendering {
bool PolylineRenderAdapter::supports(arz::cad::EntityType type) const noexcept { return type == arz::cad::EntityType::Polyline; }
bool PolylineRenderAdapter::append(const arz::cad::CadEntity& entity,
    const arz::core::Document& document, const RenderContext& context, RenderScene& scene) const {
    const auto* value = dynamic_cast<const arz::cad::PolylineEntity*>(&entity);
    const auto* layer = document.layers().get(entity.layerId());
    if (!value || !layer) return false;
    const auto resolved = arz::cad::GraphicsPropertyResolver::resolve(value->graphics(), *layer);
    scene.append({value->id(), value->vertices(), value->closed(),
        {resolved.color, resolved.lineWeight, context.isSelected(value->id())}});
    return true;
}
}
