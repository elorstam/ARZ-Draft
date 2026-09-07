#include "cad/commands/AddPolylineCommand.h"

#include <utility>

#include "cad/entities/PolylineEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

AddPolylineCommand::AddPolylineCommand(arz::core::Document& document,
    LayerId layerId, std::vector<arz::geometry::Point2D> vertices, bool closed)
    : document_(document), layerId_(layerId), vertices_(std::move(vertices)), closed_(closed) {}

bool AddPolylineCommand::execute() {
    if (inDocument_ || !document_.layers().contains(layerId_)
        || vertices_.size() < 2 || (closed_ && vertices_.size() < 3)) return false;
    if (objectId_ == arz::core::InvalidObjectId) {
        objectId_ = document_.nextObjectId();
        entity_ = std::make_unique<PolylineEntity>(objectId_, layerId_, vertices_, closed_);
        identity_ = entity_.get();
    }
    if (!entity_ || document_.contains(objectId_) || !document_.addObject(std::move(entity_))) return false;
    inDocument_ = true;
    return true;
}
bool AddPolylineCommand::undo() {
    if (!inDocument_ || document_.object(objectId_) != identity_) return false;
    entity_ = document_.takeObject(objectId_);
    if (!entity_) return false;
    inDocument_ = false;
    return true;
}
arz::core::ObjectId AddPolylineCommand::objectId() const noexcept { return objectId_; }

}
