#pragma once

#include "cad/entities/CadEntity.h"

namespace arz::core { class Document; }

namespace arz::rendering {

class RenderContext;
class RenderScene;

class IEntityRenderAdapter {
public:
    virtual ~IEntityRenderAdapter() = default;
    [[nodiscard]] virtual bool supports(arz::cad::EntityType type) const noexcept = 0;
    virtual bool append(const arz::cad::CadEntity& entity,
                        const arz::core::Document& document,
                        const RenderContext& context,
                        RenderScene& scene) const = 0;
};

}
