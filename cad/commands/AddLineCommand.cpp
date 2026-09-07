#include "cad/commands/AddLineCommand.h"

#include <utility>

#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

AddLineCommand::AddLineCommand(
    arz::core::Document& document,
    LayerId layerId,
    arz::geometry::Point2D start,
    arz::geometry::Point2D end
)
    : document_(document),
      layerId_(layerId),
      start_(start),
      end_(end) {
}

bool AddLineCommand::execute() {
    if (inDocument_) {
        return false;
    }

    if (objectId_ == arz::core::InvalidObjectId) {
        if (!document_.layers().contains(layerId_)) {
            return false;
        }

        objectId_ = document_.nextObjectId();
        entity_ = std::make_unique<LineEntity>(
            objectId_,
            layerId_,
            start_,
            end_
        );
        identity_ = entity_.get();
    }

    if (!entity_ || document_.contains(objectId_)) {
        return false;
    }

    if (!document_.addObject(std::move(entity_))) {
        return false;
    }

    inDocument_ = true;
    return true;
}

bool AddLineCommand::undo() {
    if (!inDocument_
        || document_.object(objectId_) != identity_) {
        return false;
    }

    auto entity = document_.takeObject(objectId_);

    if (!entity) {
        return false;
    }

    entity_ = std::move(entity);
    inDocument_ = false;
    return true;
}

arz::core::ObjectId
AddLineCommand::objectId() const noexcept {
    return objectId_;
}

}
