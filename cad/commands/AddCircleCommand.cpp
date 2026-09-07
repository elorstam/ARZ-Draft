#include "cad/commands/AddCircleCommand.h"

#include <cmath>
#include <utility>

#include "cad/entities/CircleEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

AddCircleCommand::AddCircleCommand(arz::core::Document& document, LayerId layerId,
    arz::geometry::Point2D center, double radius)
    : document_(document), layerId_(layerId), center_(center), radius_(radius) {}
bool AddCircleCommand::execute() {
    if (inDocument_ || !document_.layers().contains(layerId_)
        || !std::isfinite(radius_) || radius_ < 0.0) return false;
    if (objectId_ == arz::core::InvalidObjectId) {
        objectId_ = document_.nextObjectId();
        entity_ = std::make_unique<CircleEntity>(objectId_, layerId_, center_, radius_);
        identity_ = entity_.get();
    }
    if (!entity_ || document_.contains(objectId_) || !document_.addObject(std::move(entity_))) return false;
    inDocument_ = true;
    return true;
}
bool AddCircleCommand::undo() {
    if (!inDocument_ || document_.object(objectId_) != identity_) return false;
    entity_ = document_.takeObject(objectId_);
    if (!entity_) return false;
    inDocument_ = false;
    return true;
}
arz::core::ObjectId AddCircleCommand::objectId() const noexcept { return objectId_; }

}
