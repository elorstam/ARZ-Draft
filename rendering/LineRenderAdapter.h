#pragma once

#include "rendering/IEntityRenderAdapter.h"

namespace arz::rendering {

class LineRenderAdapter final : public IEntityRenderAdapter {
public:
    [[nodiscard]] bool supports(arz::cad::EntityType type) const noexcept override;
    bool append(const arz::cad::CadEntity& entity,
                const arz::core::Document& document,
                const RenderContext& context,
                RenderScene& scene) const override;
};

}
