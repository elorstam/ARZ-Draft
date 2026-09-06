#include "cad/layers/LayerTable.h"

namespace arz::cad {

LayerTable::LayerTable() {
    Layer defaultLayer(
        DefaultLayerId,
        "0"
    );

    defaultLayer.setColor(Color::white());
    defaultLayer.setLineTypeId(ContinuousLineTypeId);
    defaultLayer.setLineWeight(LineWeight::Default);

    layers_.emplace(
        DefaultLayerId,
        std::move(defaultLayer)
    );
}

bool LayerTable::contains(LayerId id) const noexcept {
    return layers_.find(id) != layers_.end();
}

bool LayerTable::containsName(
    const std::string& name
) const {
    for (const auto& [id, layer] : layers_) {
        if (layer.name() == name) {
            return true;
        }
    }

    return false;
}

Layer* LayerTable::get(LayerId id) noexcept {
    const auto it = layers_.find(id);

    if (it == layers_.end()) {
        return nullptr;
    }

    return &it->second;
}

const Layer* LayerTable::get(LayerId id) const noexcept {
    const auto it = layers_.find(id);

    if (it == layers_.end()) {
        return nullptr;
    }

    return &it->second;
}

bool LayerTable::add(Layer layer) {
    if (layer.id() == InvalidLayerId) {
        return false;
    }

    if (contains(layer.id())) {
        return false;
    }

    if (layer.name().empty()) {
        return false;
    }

    if (containsName(layer.name())) {
        return false;
    }

    layers_.emplace(
        layer.id(),
        std::move(layer)
    );

    return true;
}

bool LayerTable::remove(LayerId id) {
    if (id == DefaultLayerId) {
        return false;
    }

    return layers_.erase(id) == 1;
}

LayerId LayerTable::nextLayerId() noexcept {
    return nextId_++;
}

std::size_t LayerTable::size() const noexcept {
    return layers_.size();
}

std::vector<LayerId> LayerTable::ids() const {
    std::vector<LayerId> result;
    result.reserve(layers_.size());

    for (const auto& [id, layer] : layers_) {
        result.push_back(id);
    }

    return result;
}

}
