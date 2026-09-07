#pragma once

#include "core/commands/ICommand.h"
#include "core/objects/ObjectId.h"
#include "geometry/primitives/Point2D.h"

namespace arz::core {
class Document;
}

namespace arz::cad {

class LineEntity;

class MoveEntityCommand final : public arz::core::ICommand {
public:
    MoveEntityCommand(
        arz::core::Document& document,
        arz::core::ObjectId objectId,
        arz::geometry::Point2D delta
    );

    bool execute() override;
    bool undo() override;

private:
    [[nodiscard]] LineEntity* lineEntity() const noexcept;

    arz::core::Document& document_;
    arz::core::ObjectId objectId_;
    arz::geometry::Point2D delta_;
    arz::geometry::Point2D originalStart_{};
    arz::geometry::Point2D originalEnd_{};
    arz::geometry::Point2D movedStart_{};
    arz::geometry::Point2D movedEnd_{};
    LineEntity* identity_{nullptr};
    bool initialized_{false};
    bool applied_{false};
};

}
