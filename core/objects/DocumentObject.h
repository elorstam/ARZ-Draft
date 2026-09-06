#pragma once

#include "core/objects/ObjectId.h"

namespace arz::core {

enum class ObjectType {
    Unknown = 0,
    Generic
};

class DocumentObject {
public:
    explicit DocumentObject(ObjectId id);
    virtual ~DocumentObject() = default;

    [[nodiscard]] ObjectId id() const noexcept;

    [[nodiscard]] virtual ObjectType type() const noexcept {
        return ObjectType::Generic;
    }

private:
    ObjectId id_{InvalidObjectId};
};

}
