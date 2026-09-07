#pragma once

#include <cstddef>
#include <vector>

#include "cad/entities/EntityGraphicsProperties.h"
#include "cad/layers/LayerId.h"
#include "geometry/primitives/Point2D.h"
#include "core/objects/ObjectId.h"

namespace arz::core { class Document; }

namespace arz::interaction {

struct ClipboardLine final {
    arz::cad::LayerId layerId{arz::cad::DefaultLayerId};
    arz::geometry::Point2D start{};
    arz::geometry::Point2D end{};
    arz::cad::EntityGraphicsProperties graphics{};
};

struct ClipboardPolyline final {
    arz::cad::LayerId layerId{arz::cad::DefaultLayerId};
    std::vector<arz::geometry::Point2D> vertices;
    bool closed{};
    arz::cad::EntityGraphicsProperties graphics{};
};

struct ClipboardCircle final {
    arz::cad::LayerId layerId{arz::cad::DefaultLayerId};
    arz::geometry::Point2D center{};
    double radius{};
    arz::cad::EntityGraphicsProperties graphics{};
};

struct ClipboardArc final {
    arz::cad::LayerId layerId{arz::cad::DefaultLayerId};
    arz::geometry::Point2D center{};
    double radius{};
    double startAngle{};
    double endAngle{};
    bool counterClockwise{true};
    arz::cad::EntityGraphicsProperties graphics{};
};

class CadClipboard final {
public:
    bool copy(const arz::core::Document& document,
              const std::vector<arz::core::ObjectId>& ids);
    void clear() noexcept;
    [[nodiscard]] const std::vector<ClipboardLine>& lines() const noexcept;
    [[nodiscard]] const std::vector<ClipboardPolyline>& polylines() const noexcept;
    [[nodiscard]] const std::vector<ClipboardCircle>& circles() const noexcept;
    [[nodiscard]] const std::vector<ClipboardArc>& arcs() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] arz::geometry::Point2D basePoint() const noexcept;

private:
    std::vector<ClipboardLine> lines_;
    std::vector<ClipboardPolyline> polylines_;
    std::vector<ClipboardCircle> circles_;
    std::vector<ClipboardArc> arcs_;
    arz::geometry::Point2D basePoint_{};
};

}
