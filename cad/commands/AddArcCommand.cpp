#include "cad/commands/AddArcCommand.h"

#include <cmath>
#include <utility>

#include "cad/entities/ArcEntity.h"
#include "core/document/Document.h"

namespace arz::cad {

AddArcCommand::AddArcCommand(arz::core::Document& document, LayerId layerId,
    arz::geometry::Point2D center, double radius, double startAngle,
    double endAngle, bool counterClockwise)
    : document_(document), layerId_(layerId), center_(center), radius_(radius),
      startAngle_(startAngle), endAngle_(endAngle), counterClockwise_(counterClockwise) {}
bool AddArcCommand::execute() {
    if (inDocument_ || !document_.layers().contains(layerId_)
        || !std::isfinite(radius_) || radius_ <= 0.0
        || !std::isfinite(startAngle_) || !std::isfinite(endAngle_)) return false;
    if (objectId_ == arz::core::InvalidObjectId) {
        objectId_ = document_.nextObjectId();
        entity_ = std::make_unique<ArcEntity>(objectId_, layerId_, center_, radius_,
                                              startAngle_, endAngle_, counterClockwise_);
        identity_ = entity_.get();
    }
    if (!entity_ || document_.contains(objectId_) || !document_.addObject(std::move(entity_))) return false;
    inDocument_ = true;
    return true;
}
bool AddArcCommand::undo() {
    if (!inDocument_ || document_.object(objectId_) != identity_) return false;
    entity_ = document_.takeObject(objectId_);
    if (!entity_) return false;
    inDocument_ = false;
    return true;
}
arz::core::ObjectId AddArcCommand::objectId() const noexcept { return objectId_; }

}
