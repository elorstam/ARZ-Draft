#include "cad/commands/MoveEntityCommand.h"

#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

MoveEntityCommand::MoveEntityCommand(
    arz::core::Document& document,
    arz::core::ObjectId objectId,
    arz::geometry::Point2D delta
)
    : document_(document),
      objectId_(objectId),
      delta_(delta) {
}

bool MoveEntityCommand::execute() {
    if (applied_
        || objectId_ == arz::core::InvalidObjectId) {
        return false;
    }

    auto* line = lineEntity();

    if (line == nullptr) {
        return false;
    }

    if (!initialized_) {
        identity_ = line;
        originalStart_ = line->start();
        originalEnd_ = line->end();
        movedStart_ = {
            originalStart_.x + delta_.x,
            originalStart_.y + delta_.y
        };
        movedEnd_ = {
            originalEnd_.x + delta_.x,
            originalEnd_.y + delta_.y
        };
        initialized_ = true;
    } else if (line != identity_
        || line->start() != originalStart_
        || line->end() != originalEnd_) {
        return false;
    }

    line->setStart(movedStart_);
    line->setEnd(movedEnd_);
    applied_ = true;
    return true;
}

bool MoveEntityCommand::undo() {
    auto* line = lineEntity();

    if (!applied_
        || line == nullptr
        || line != identity_
        || line->start() != movedStart_
        || line->end() != movedEnd_) {
        return false;
    }

    line->setStart(originalStart_);
    line->setEnd(originalEnd_);
    applied_ = false;
    return true;
}

LineEntity* MoveEntityCommand::lineEntity() const noexcept {
    return dynamic_cast<LineEntity*>(
        document_.object(objectId_)
    );
}

}
