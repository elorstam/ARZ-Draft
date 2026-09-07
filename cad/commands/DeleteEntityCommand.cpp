#include "cad/commands/DeleteEntityCommand.h"

#include <utility>

#include "cad/entities/CadEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

DeleteEntityCommand::DeleteEntityCommand(
    arz::core::Document& document,
    arz::core::ObjectId objectId
)
    : document_(document),
      objectId_(objectId) {
}

bool DeleteEntityCommand::execute() {
    if (deleted_
        || objectId_ == arz::core::InvalidObjectId) {
        return false;
    }

    auto* object = document_.object(objectId_);

    if (dynamic_cast<CadEntity*>(object) == nullptr) {
        return false;
    }

    if (identity_ != nullptr && object != identity_) {
        return false;
    }

    auto entity = document_.takeObject(objectId_);

    if (!entity) {
        return false;
    }

    if (identity_ == nullptr) {
        identity_ = entity.get();
    }

    entity_ = std::move(entity);
    deleted_ = true;
    return true;
}

bool DeleteEntityCommand::undo() {
    if (!deleted_ || !entity_) {
        return false;
    }

    if (!document_.addObject(std::move(entity_))) {
        return false;
    }

    deleted_ = false;
    return true;
}

}
