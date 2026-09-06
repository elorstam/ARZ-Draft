#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "cad/layers/Layer.h"

namespace arz::cad {

class LayerTable final {
public:
    LayerTable();

    [[nodiscard]] bool contains(LayerId id) const noexcept;
    [[nodiscard]] bool containsName(const std::string& name) const;

    [[nodiscard]] Layer* get(LayerId id) noexcept;
    [[nodiscard]] const Layer* get(LayerId id) const noexcept;

    bool add(Layer layer);
    bool remove(LayerId id);

    [[nodiscard]] LayerId nextLayerId() noexcept;

    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] std::vector<LayerId> ids() const;

private:
    LayerId nextId_{2};

    std::unordered_map<LayerId, Layer> layers_;
};

}
