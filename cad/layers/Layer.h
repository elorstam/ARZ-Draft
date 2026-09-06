#pragma once

#include <string>

#include "cad/layers/Color.h"
#include "cad/layers/LayerId.h"
#include "cad/layers/LineTypeId.h"
#include "cad/layers/LineWeight.h"

namespace arz::cad {

class Layer final {
public:
    Layer(
        LayerId id,
        std::string name
    );

    [[nodiscard]] LayerId id() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;

    [[nodiscard]] const Color& color() const noexcept;
    [[nodiscard]] LineTypeId lineTypeId() const noexcept;
    [[nodiscard]] LineWeight lineWeight() const noexcept;

    [[nodiscard]] bool visible() const noexcept;
    [[nodiscard]] bool frozen() const noexcept;
    [[nodiscard]] bool locked() const noexcept;
    [[nodiscard]] bool plottable() const noexcept;

    void setName(std::string name);
    void setColor(Color color) noexcept;
    void setLineTypeId(LineTypeId id) noexcept;
    void setLineWeight(LineWeight value) noexcept;

    void setVisible(bool value) noexcept;
    void setFrozen(bool value) noexcept;
    void setLocked(bool value) noexcept;
    void setPlottable(bool value) noexcept;

private:
    LayerId id_{InvalidLayerId};
    std::string name_;

    Color color_{Color::white()};
    LineTypeId lineTypeId_{ContinuousLineTypeId};
    LineWeight lineWeight_{LineWeight::Default};

    bool visible_{true};
    bool frozen_{false};
    bool locked_{false};
    bool plottable_{true};
};

}
