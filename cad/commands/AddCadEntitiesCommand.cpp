#include "cad/commands/AddCadEntitiesCommand.h"

#include <cmath>
#include <utility>

#include "cad/entities/ArcEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/LineEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "core/document/Document.h"

namespace arz::cad {
namespace {
bool valid(const CadEntityCreationData& item, const arz::core::Document& document) {
    return std::visit([&document](const auto& value) {
        if (!document.layers().contains(value.layerId)) return false;
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, PolylineCreationData>)
            return value.vertices.size() >= 2 && (!value.closed || value.vertices.size() >= 3);
        if constexpr (std::is_same_v<T, CircleCreationData>)
            return std::isfinite(value.radius) && value.radius >= 0.0;
        if constexpr (std::is_same_v<T, ArcCreationData>)
            return std::isfinite(value.radius) && value.radius > 0.0
                && std::isfinite(value.startAngle) && std::isfinite(value.endAngle);
        return true;
    }, item);
}

std::unique_ptr<arz::core::DocumentObject> create(arz::core::ObjectId id,
                                                   const CadEntityCreationData& item) {
    return std::visit([id](const auto& value) -> std::unique_ptr<arz::core::DocumentObject> {
        using T = std::decay_t<decltype(value)>;
        std::unique_ptr<CadEntity> entity;
        if constexpr (std::is_same_v<T, LineCreationData>)
            entity = std::make_unique<LineEntity>(id, value.layerId, value.start, value.end);
        else if constexpr (std::is_same_v<T, PolylineCreationData>)
            entity = std::make_unique<PolylineEntity>(id, value.layerId, value.vertices, value.closed);
        else if constexpr (std::is_same_v<T, CircleCreationData>)
            entity = std::make_unique<CircleEntity>(id, value.layerId, value.center, value.radius);
        else
            entity = std::make_unique<ArcEntity>(id, value.layerId, value.center, value.radius,
                                                  value.startAngle, value.endAngle, value.counterClockwise);
        entity->graphics() = value.graphics;
        return entity;
    }, item);
}
}

AddCadEntitiesCommand::AddCadEntitiesCommand(arz::core::Document& document,
    std::vector<CadEntityCreationData> entities)
    : document_(document), data_(std::move(entities)) {}
bool AddCadEntitiesCommand::execute() {
    if (inDocument_ || data_.empty()) return false;
    for (const auto& item : data_) if (!valid(item, document_)) return false;
    if (objectIds_.empty()) {
        objectIds_.reserve(data_.size()); entities_.reserve(data_.size()); identities_.reserve(data_.size());
        for (const auto& item : data_) {
            const auto id = document_.nextObjectId();
            auto entity = create(id, item);
            objectIds_.push_back(id); identities_.push_back(entity.get()); entities_.push_back(std::move(entity));
        }
    }
    for (const auto id : objectIds_) if (document_.contains(id)) return false;
    for (auto& entity : entities_) if (!document_.addObject(std::move(entity))) return false;
    entities_.clear(); inDocument_ = true; return true;
}
bool AddCadEntitiesCommand::undo() {
    if (!inDocument_) return false;
    for (std::size_t i = 0; i < objectIds_.size(); ++i)
        if (document_.object(objectIds_[i]) != identities_[i]) return false;
    for (const auto id : objectIds_) entities_.push_back(document_.takeObject(id));
    inDocument_ = false; return true;
}
const std::vector<arz::core::ObjectId>& AddCadEntitiesCommand::objectIds() const noexcept { return objectIds_; }

}
