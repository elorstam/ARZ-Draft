#include "interaction/drafting/DraftingSettings.h"

namespace arz::interaction {

bool DraftingSettings::enabled(DraftingToggle toggle) const noexcept {
    switch (toggle) {
    case DraftingToggle::ObjectSnap: return objectSnap_;
    case DraftingToggle::GridDisplay: return gridDisplay_;
    case DraftingToggle::Ortho: return ortho_;
    case DraftingToggle::GridSnap: return gridSnap_;
    case DraftingToggle::PolarTracking: return polarTracking_;
    case DraftingToggle::ObjectSnapTracking: return objectSnapTracking_;
    case DraftingToggle::DynamicInput: return dynamicInput_;
    }
    return false;
}

bool DraftingSettings::setEnabled(
    DraftingToggle toggle,
    bool value
) noexcept {
    switch (toggle) {
    case DraftingToggle::ObjectSnap: objectSnap_ = value; break;
    case DraftingToggle::GridDisplay: gridDisplay_ = value; break;
    case DraftingToggle::Ortho:
        ortho_ = value;
        if (value) polarTracking_ = false;
        break;
    case DraftingToggle::GridSnap: gridSnap_ = value; break;
    case DraftingToggle::PolarTracking:
        polarTracking_ = value;
        if (value) ortho_ = false;
        break;
    case DraftingToggle::ObjectSnapTracking: objectSnapTracking_ = value; break;
    case DraftingToggle::DynamicInput: dynamicInput_ = value; break;
    }
    return enabled(toggle);
}

bool DraftingSettings::toggle(DraftingToggle toggle) noexcept {
    return setEnabled(toggle, !enabled(toggle));
}

}
