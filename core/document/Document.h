#pragma once

#include <memory>
#include <vector>

#include "cad/layers/LayerTable.h"
#include "core/document/ObjectRegistry.h"

namespace arz::core {

class Document final {
public:
    Document() = default;

    [[nodiscard]] ObjectId nextObjectId() noexcept;

    bool addObject(std::unique_ptr<DocumentObject>&& object);
    bool removeObject(ObjectId id);
    [[nodiscard]] std::unique_ptr<DocumentObject>
    takeObject(ObjectId id) noexcept;

    [[nodiscard]] DocumentObject* object(ObjectId id) noexcept;
    [[nodiscard]] const DocumentObject* object(ObjectId id) const noexcept;

    [[nodiscard]] bool contains(ObjectId id) const noexcept;
    [[nodiscard]] std::size_t objectCount() const noexcept;
    [[nodiscard]] std::vector<ObjectId> objectIds() const;

    [[nodiscard]] arz::cad::LayerTable& layers() noexcept;
    [[nodiscard]] const arz::cad::LayerTable& layers() const noexcept;

private:
    ObjectId nextId_{1};

    ObjectRegistry registry_;
    arz::cad::LayerTable layers_;
};

}
