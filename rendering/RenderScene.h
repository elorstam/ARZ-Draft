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

struct PolylineRenderPrimitive final {
    arz::core::ObjectId objectId{arz::core::InvalidObjectId};
    std::vector<arz::geometry::Point2D> vertices;
    bool closed{};
    RenderStyle style{};
};

struct CircleRenderPrimitive final {
    arz::core::ObjectId objectId{arz::core::InvalidObjectId};
    arz::geometry::Point2D center{};
    double radius{};
    RenderStyle style{};
};

struct ArcRenderPrimitive final {
    arz::core::ObjectId objectId{arz::core::InvalidObjectId};
    arz::geometry::Point2D center{};
    double radius{};
    double startAngle{};
    double sweepAngle{};
    bool counterClockwise{true};
    RenderStyle style{};
};

using RenderPrimitive = std::variant<LineRenderPrimitive, PolylineRenderPrimitive,
                                     CircleRenderPrimitive, ArcRenderPrimitive>;

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
    void append(PolylineRenderPrimitive primitive) { primitives_.emplace_back(std::move(primitive)); }
    void append(CircleRenderPrimitive primitive) { primitives_.emplace_back(std::move(primitive)); }
    void append(ArcRenderPrimitive primitive) { primitives_.emplace_back(std::move(primitive)); }

    [[nodiscard]] const std::vector<RenderPrimitive>& primitives() const noexcept {
        return primitives_;
    }

private:
    std::vector<RenderPrimitive> primitives_;
};

}
