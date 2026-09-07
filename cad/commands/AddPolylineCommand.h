#pragma once

#include <memory>
#include <vector>

#include "cad/layers/LayerId.h"
#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"
#include "geometry/primitives/Point2D.h"

namespace arz::core { class Document; }

namespace arz::cad {

class AddPolylineCommand final : public arz::core::ICommand {
public:
    AddPolylineCommand(arz::core::Document& document, LayerId layerId,
                       std::vector<arz::geometry::Point2D> vertices, bool closed);
    bool execute() override;
    bool undo() override;
    [[nodiscard]] arz::core::ObjectId objectId() const noexcept;

private:
    arz::core::Document& document_;
    LayerId layerId_;
    std::vector<arz::geometry::Point2D> vertices_;
    bool closed_{};
    arz::core::ObjectId objectId_{arz::core::InvalidObjectId};
    std::unique_ptr<arz::core::DocumentObject> entity_;
    arz::core::DocumentObject* identity_{};
    bool inDocument_{};
};

}
