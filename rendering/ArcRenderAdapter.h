#pragma once
#include "rendering/IEntityRenderAdapter.h"
namespace arz::rendering {
class ArcRenderAdapter final : public IEntityRenderAdapter {
public:
    [[nodiscard]] bool supports(arz::cad::EntityType type) const noexcept override;
    bool append(const arz::cad::CadEntity&, const arz::core::Document&,
        const RenderContext&, RenderScene&) const override;
};
}
