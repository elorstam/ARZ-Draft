#include "core/objects/DocumentObject.h"

namespace arz::core {

DocumentObject::DocumentObject(ObjectId id)
    : id_(id) {
}

ObjectId DocumentObject::id() const noexcept {
    return id_;
}

}
