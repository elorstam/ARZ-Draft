#include "rendering/LineRenderAdapter.h"

#include "cad/entities/GraphicsPropertyResolver.h"
#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"
#include "rendering/RenderContext.h"
#include "rendering/RenderScene.h"

namespace arz::rendering {

bool LineRenderAdapter::supports(arz::cad::EntityType type) const noexcept {
    return type == arz::cad::EntityType::Line;
}

bool LineRenderAdapter::append(
    const arz::cad::CadEntity& entity,
    const arz::core::Document& document,
    const RenderContext& context,
    RenderScene& scene
) const {
    const auto* line = dynamic_cast<const arz::cad::LineEntity*>(&entity);
    const auto* layer = document.layers().get(entity.layerId());
    if (line == nullptr || layer == nullptr) return false;
    const auto resolved = arz::cad::GraphicsPropertyResolver::resolve(
        line->graphics(), *layer);
    scene.append({
        line->id(),
        line->start(),
        line->end(),
        {resolved.color, resolved.lineWeight, context.isSelected(line->id())}
    });
    return true;
}

}
