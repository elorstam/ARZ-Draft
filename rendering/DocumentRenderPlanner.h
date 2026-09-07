#pragma once

#include <optional>

#include "geometry/primitives/BoundingBox2D.h"
#include "rendering/LineRenderAdapter.h"

namespace arz::core { class Document; }

namespace arz::rendering {

class RenderContext;
class RenderScene;

class DocumentRenderPlanner final {
public:
    void build(const arz::core::Document& document,
               const RenderContext& context,
               RenderScene& scene) const;
    [[nodiscard]] std::optional<arz::geometry::BoundingBox2D>
    visibleDocumentBounds(const arz::core::Document& document) const;

private:
    LineRenderAdapter lineAdapter_;
};

}
