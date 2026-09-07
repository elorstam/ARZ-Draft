#include "cad/commands/AddLinesCommand.h"

#include <utility>

#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

AddLinesCommand::AddLinesCommand(
    arz::core::Document& document,
    std::vector<LineCreationData> lines
)
    : document_(document), lines_(std::move(lines)) {
}

bool AddLinesCommand::execute() {
    if (inDocument_ || lines_.empty()) return false;
    for (const auto& line : lines_) {
        if (!document_.layers().contains(line.layerId)) return false;
    }
    if (objectIds_.empty()) {
        for (const auto& data : lines_) {
            const auto id = document_.nextObjectId();
            auto line = std::make_unique<LineEntity>(id, data.layerId, data.start, data.end);
            line->graphics() = data.graphics;
            objectIds_.push_back(id);
            identities_.push_back(line.get());
            entities_.push_back(std::move(line));
        }
    }
    for (const auto id : objectIds_) if (document_.contains(id)) return false;
    for (auto& entity : entities_) {
        if (!document_.addObject(std::move(entity))) return false;
    }
    entities_.clear();
    inDocument_ = true;
    return true;
}

bool AddLinesCommand::undo() {
    if (!inDocument_) return false;
    for (std::size_t index = 0; index < objectIds_.size(); ++index) {
        if (document_.object(objectIds_[index]) != identities_[index]) return false;
    }
    for (const auto id : objectIds_) entities_.push_back(document_.takeObject(id));
    inDocument_ = false;
    return true;
}

const std::vector<arz::core::ObjectId>& AddLinesCommand::objectIds() const noexcept {
    return objectIds_;
}

}
