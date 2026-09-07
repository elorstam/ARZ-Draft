#include "core/document/Document.h"

#include <utility>

namespace arz::core {

ObjectId Document::nextObjectId() noexcept {
    return nextId_++;
}

bool Document::addObject(
    std::unique_ptr<DocumentObject>&& object
) {
    return registry_.add(std::move(object));
}

bool Document::removeObject(ObjectId id) {
    return registry_.remove(id);
}

std::unique_ptr<DocumentObject>
Document::takeObject(ObjectId id) noexcept {
    return registry_.take(id);
}

DocumentObject* Document::object(ObjectId id) noexcept {
    return registry_.get(id);
}

const DocumentObject* Document::object(
    ObjectId id
) const noexcept {
    return registry_.get(id);
}

bool Document::contains(ObjectId id) const noexcept {
    return registry_.contains(id);
}

std::size_t Document::objectCount() const noexcept {
    return registry_.size();
}

std::vector<ObjectId> Document::objectIds() const {
    return registry_.ids();
}

arz::cad::LayerTable& Document::layers() noexcept {
    return layers_;
}

const arz::cad::LayerTable&
Document::layers() const noexcept {
    return layers_;
}

}
