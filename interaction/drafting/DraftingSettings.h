#pragma once

namespace arz::interaction {

enum class DraftingToggle {
    ObjectSnap,
    GridDisplay,
    Ortho,
    GridSnap,
    PolarTracking,
    ObjectSnapTracking,
    DynamicInput
};

class DraftingSettings final {
public:
    [[nodiscard]] bool enabled(DraftingToggle toggle) const noexcept;
    bool setEnabled(DraftingToggle toggle, bool enabled) noexcept;
    bool toggle(DraftingToggle toggle) noexcept;

private:
    bool objectSnap_{true};
    bool gridDisplay_{false};
    bool ortho_{false};
    bool gridSnap_{false};
    bool polarTracking_{false};
    bool objectSnapTracking_{false};
    bool dynamicInput_{true};
};

}
