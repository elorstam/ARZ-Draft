#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "cad/commands/AddLinesCommand.h"
#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"

namespace arz::core { class Document; }
namespace arz::cad {

struct PolylineCreationData final {
    LayerId layerId{DefaultLayerId};
    std::vector<arz::geometry::Point2D> vertices;
    bool closed{};
    EntityGraphicsProperties graphics{};
};
struct CircleCreationData final {
    LayerId layerId{DefaultLayerId};
    arz::geometry::Point2D center{};
    double radius{};
    EntityGraphicsProperties graphics{};
};
struct ArcCreationData final {
    LayerId layerId{DefaultLayerId};
    arz::geometry::Point2D center{};
    double radius{};
    double startAngle{};
    double endAngle{};
    bool counterClockwise{true};
    EntityGraphicsProperties graphics{};
};
using CadEntityCreationData = std::variant<LineCreationData, PolylineCreationData,
                                           CircleCreationData, ArcCreationData>;

class AddCadEntitiesCommand final : public arz::core::ICommand {
public:
    AddCadEntitiesCommand(arz::core::Document& document,
                          std::vector<CadEntityCreationData> entities);
    bool execute() override;
    bool undo() override;
    [[nodiscard]] const std::vector<arz::core::ObjectId>& objectIds() const noexcept;
private:
    arz::core::Document& document_;
    std::vector<CadEntityCreationData> data_;
    std::vector<arz::core::ObjectId> objectIds_;
    std::vector<std::unique_ptr<arz::core::DocumentObject>> entities_;
    std::vector<arz::core::DocumentObject*> identities_;
    bool inDocument_{};
};

}
