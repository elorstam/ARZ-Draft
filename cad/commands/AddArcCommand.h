#pragma once

#include <memory>

#include "cad/layers/LayerId.h"
#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"
#include "geometry/primitives/Point2D.h"

namespace arz::core { class Document; }
namespace arz::cad {

class AddArcCommand final : public arz::core::ICommand {
public:
    AddArcCommand(arz::core::Document& document, LayerId layerId,
                  arz::geometry::Point2D center, double radius,
                  double startAngle, double endAngle, bool counterClockwise);
    bool execute() override;
    bool undo() override;
    [[nodiscard]] arz::core::ObjectId objectId() const noexcept;
private:
    arz::core::Document& document_;
    LayerId layerId_;
    arz::geometry::Point2D center_{};
    double radius_{};
    double startAngle_{};
    double endAngle_{};
    bool counterClockwise_{true};
    arz::core::ObjectId objectId_{arz::core::InvalidObjectId};
    std::unique_ptr<arz::core::DocumentObject> entity_;
    arz::core::DocumentObject* identity_{};
    bool inDocument_{};
};

}
