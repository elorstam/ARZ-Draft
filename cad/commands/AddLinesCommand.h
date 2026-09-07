#pragma once

#include <memory>
#include <vector>

#include "cad/entities/EntityGraphicsProperties.h"
#include "cad/layers/LayerId.h"
#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"
#include "geometry/primitives/Point2D.h"

namespace arz::core { class Document; }

namespace arz::cad {

struct LineCreationData final {
    LayerId layerId{DefaultLayerId};
    arz::geometry::Point2D start{};
    arz::geometry::Point2D end{};
    EntityGraphicsProperties graphics{};
};

class AddLinesCommand final : public arz::core::ICommand {
public:
    AddLinesCommand(arz::core::Document& document,
                    std::vector<LineCreationData> lines);
    bool execute() override;
    bool undo() override;
    [[nodiscard]] const std::vector<arz::core::ObjectId>& objectIds() const noexcept;

private:
    arz::core::Document& document_;
    std::vector<LineCreationData> lines_;
    std::vector<arz::core::ObjectId> objectIds_;
    std::vector<std::unique_ptr<arz::core::DocumentObject>> entities_;
    std::vector<arz::core::DocumentObject*> identities_;
    bool inDocument_{false};
};

}
