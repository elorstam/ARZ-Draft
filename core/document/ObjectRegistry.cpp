#include "core/document/ObjectRegistry.h"

namespace arz::core {

bool ObjectRegistry::contains(ObjectId id) const noexcept {
    return objects_.find(id) != objects_.end();
}

DocumentObject* ObjectRegistry::get(ObjectId id) noexcept {
    const auto it = objects_.find(id);

    if (it == objects_.end()) {
        return nullptr;
    }

    return it->second.get();
}

const DocumentObject* ObjectRegistry::get(ObjectId id) const noexcept {
    const auto it = objects_.find(id);

    if (it == objects_.end()) {
        return nullptr;
    }

    return it->second.get();
}

bool ObjectRegistry::add(std::unique_ptr<DocumentObject> object) {
    if (!object) {
        return false;
    }

    const auto id = object->id();

    if (id == InvalidObjectId || contains(id)) {
        return false;
    }

    objects_.emplace(id, std::move(object));
    return true;
}

bool ObjectRegistry::remove(ObjectId id) {
    return objects_.erase(id) == 1;
}

std::size_t ObjectRegistry::size() const noexcept {
    return objects_.size();
}

bool ObjectRegistry::empty() const noexcept {
    return objects_.empty();
}

std::vector<ObjectId> ObjectRegistry::ids() const {
    std::vector<ObjectId> result;
    result.reserve(objects_.size());

    for (const auto& [id, object] : objects_) {
        result.push_back(id);
    }

    return result;
}

}
