#pragma once

#include <cstddef>
#include <vector>

#include "core/objects/ObjectId.h"

namespace arz::core { class Document; }

namespace arz::interaction {

class SelectionSet final {
public:
    [[nodiscard]] bool contains(arz::core::ObjectId id) const noexcept;
    bool add(arz::core::ObjectId id);
    bool remove(arz::core::ObjectId id) noexcept;
    bool toggle(arz::core::ObjectId id);
    void addAll(const std::vector<arz::core::ObjectId>& ids);
    void removeAll(const std::vector<arz::core::ObjectId>& ids) noexcept;
    void replace(std::vector<arz::core::ObjectId> ids);
    void clear() noexcept;
    void removeStale(const arz::core::Document& document);
    [[nodiscard]] const std::vector<arz::core::ObjectId>& ids() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

private:
    std::vector<arz::core::ObjectId> ids_;
};

}
