#pragma once

#include <optional>
#include <string>

#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

struct SelectionWindowOverlay final {
    arz::geometry::Point2D first{};
    arz::geometry::Point2D current{};
    bool crossing{false};
};

class OverlayState final {
public:
    void setDynamicText(std::string text);
    [[nodiscard]] const std::string& dynamicText() const noexcept;
    void beginSelectionWindow(arz::geometry::Point2D point);
    void updateSelectionWindow(arz::geometry::Point2D point);
    void clearSelectionWindow() noexcept;
    [[nodiscard]] const std::optional<SelectionWindowOverlay>& selectionWindow() const noexcept;

private:
    std::string dynamicText_;
    std::optional<SelectionWindowOverlay> selectionWindow_;
};

}
