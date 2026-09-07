#include "cad/commands/DeleteEntitiesCommand.h"

#include <algorithm>
#include <utility>

#include "cad/entities/CadEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

DeleteEntitiesCommand::DeleteEntitiesCommand(
    arz::core::Document& document,
    std::vector<arz::core::ObjectId> objectIds
)
    : document_(document), objectIds_(std::move(objectIds)) {
    std::ranges::sort(objectIds_);
    objectIds_.erase(std::unique(objectIds_.begin(), objectIds_.end()), objectIds_.end());
}

bool DeleteEntitiesCommand::execute() {
    if (deleted_ || objectIds_.empty()) return false;
    for (std::size_t index = 0; index < objectIds_.size(); ++index) {
        auto* object = document_.object(objectIds_[index]);
        if (dynamic_cast<CadEntity*>(object) == nullptr
            || (!identities_.empty() && identities_[index] != object)) {
            return false;
        }
    }

    entities_.clear();
    for (const auto id : objectIds_) {
        entities_.push_back(document_.takeObject(id));
    }
    if (identities_.empty()) {
        for (const auto& entity : entities_) identities_.push_back(entity.get());
    }
    deleted_ = true;
    return true;
}

bool DeleteEntitiesCommand::undo() {
    if (!deleted_ || entities_.size() != objectIds_.size()) return false;
    for (const auto id : objectIds_) {
        if (document_.contains(id)) return false;
    }
    for (auto& entity : entities_) {
        if (!document_.addObject(std::move(entity))) return false;
    }
    entities_.clear();
    deleted_ = false;
    return true;
}

}
