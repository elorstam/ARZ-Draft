#pragma once

#include <optional>
#include <string>
#include <vector>
#include <cstddef>

#include "geometry/primitives/Point2D.h"

namespace arz::interaction {

struct SelectionWindowOverlay final {
    arz::geometry::Point2D first{};
    arz::geometry::Point2D current{};
    bool crossing{false};
};

struct TransientLine final {
    arz::geometry::Point2D start{};
    arz::geometry::Point2D end{};
};

struct PastePlacementOverlay final {
    arz::geometry::Point2D basePoint{};
    arz::geometry::Point2D insertionPoint{};
    std::vector<TransientLine> lines;
};

class OverlayState final {
public:
    void setDynamicText(std::string text);
    [[nodiscard]] const std::string& dynamicText() const noexcept;
    void beginSelectionWindow(arz::geometry::Point2D point);
    void updateSelectionWindow(arz::geometry::Point2D point);
    void clearSelectionWindow() noexcept;
    [[nodiscard]] const std::optional<SelectionWindowOverlay>& selectionWindow() const noexcept;
    void setCommandSuggestions(std::vector<std::string> suggestions);
    void selectPreviousSuggestion() noexcept;
    void selectNextSuggestion() noexcept;
    [[nodiscard]] const std::vector<std::string>& commandSuggestions() const noexcept;
    [[nodiscard]] std::size_t selectedSuggestionIndex() const noexcept;
    [[nodiscard]] std::optional<std::string> selectedSuggestion() const;
    void setPastePlacement(PastePlacementOverlay placement);
    void clearPastePlacement() noexcept;
    [[nodiscard]] const std::optional<PastePlacementOverlay>& pastePlacement() const noexcept;

private:
    std::string dynamicText_;
    std::optional<SelectionWindowOverlay> selectionWindow_;
    std::vector<std::string> commandSuggestions_;
    std::size_t selectedSuggestionIndex_{0};
    std::optional<PastePlacementOverlay> pastePlacement_;
};

}
