#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "core/objects/DocumentObject.h"

namespace arz::core {

class ObjectRegistry final {
public:
    [[nodiscard]] bool contains(ObjectId id) const noexcept;

    [[nodiscard]] DocumentObject* get(ObjectId id) noexcept;
    [[nodiscard]] const DocumentObject* get(ObjectId id) const noexcept;

    bool add(std::unique_ptr<DocumentObject> object);
    bool remove(ObjectId id);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    [[nodiscard]] std::vector<ObjectId> ids() const;

private:
    std::unordered_map<ObjectId, std::unique_ptr<DocumentObject>> objects_;
};

}
