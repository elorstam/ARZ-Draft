#include "interaction/selection/SelectionSet.h"

#include <algorithm>

#include "core/document/Document.h"

namespace arz::interaction {

bool SelectionSet::contains(arz::core::ObjectId id) const noexcept {
    return std::ranges::find(ids_, id) != ids_.end();
}

bool SelectionSet::add(arz::core::ObjectId id) {
    if (id == arz::core::InvalidObjectId || contains(id)) {
        return false;
    }
    ids_.push_back(id);
    return true;
}

bool SelectionSet::remove(arz::core::ObjectId id) noexcept {
    const auto iterator = std::ranges::find(ids_, id);
    if (iterator == ids_.end()) {
        return false;
    }
    ids_.erase(iterator);
    return true;
}

bool SelectionSet::toggle(arz::core::ObjectId id) {
    return contains(id) ? remove(id) : add(id);
}

void SelectionSet::addAll(const std::vector<arz::core::ObjectId>& ids) {
    for (const auto id : ids) {
        add(id);
    }
}

void SelectionSet::removeAll(
    const std::vector<arz::core::ObjectId>& ids
) noexcept {
    for (const auto id : ids) {
        remove(id);
    }
}

void SelectionSet::replace(std::vector<arz::core::ObjectId> ids) {
    clear();
    addAll(ids);
}

void SelectionSet::clear() noexcept { ids_.clear(); }

void SelectionSet::removeStale(const arz::core::Document& document) {
    std::erase_if(ids_, [&document](auto id) { return !document.contains(id); });
}

const std::vector<arz::core::ObjectId>& SelectionSet::ids() const noexcept {
    return ids_;
}

std::size_t SelectionSet::size() const noexcept { return ids_.size(); }
bool SelectionSet::empty() const noexcept { return ids_.empty(); }

}
