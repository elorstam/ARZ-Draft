#pragma once

#include <variant>
#include <vector>
#include <cstddef>
#include <utility>

#include "cad/layers/Color.h"
#include "cad/layers/LineWeight.h"
#include "core/objects/ObjectId.h"
#include "geometry/primitives/Point2D.h"

namespace arz::rendering {

struct RenderStyle final {
    arz::cad::Color color{};
    arz::cad::LineWeight lineWeight{arz::cad::LineWeight::Default};
    bool selected{false};
};

struct LineRenderPrimitive final {
    arz::core::ObjectId objectId{arz::core::InvalidObjectId};
    arz::geometry::Point2D start{};
    arz::geometry::Point2D end{};
    RenderStyle style{};
};

using RenderPrimitive = std::variant<LineRenderPrimitive>;

class RenderScene final {
public:
    void prepare(std::size_t expectedPrimitiveCount) {
        primitives_.clear();
        if (primitives_.capacity() < expectedPrimitiveCount) {
            primitives_.reserve(expectedPrimitiveCount);
        }
    }

    void append(LineRenderPrimitive primitive) {
        primitives_.emplace_back(std::move(primitive));
    }

    [[nodiscard]] const std::vector<RenderPrimitive>& primitives() const noexcept {
        return primitives_;
    }

private:
    std::vector<RenderPrimitive> primitives_;
};

}
