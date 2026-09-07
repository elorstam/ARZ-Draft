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

struct TransientPolyline final {
    std::vector<arz::geometry::Point2D> vertices;
    bool closed{};
};

struct TransientCircle final {
    arz::geometry::Point2D center{};
    double radius{};
};

struct TransientArc final {
    arz::geometry::Point2D center{};
    double radius{};
    double startAngle{};
    double sweepAngle{};
    bool counterClockwise{true};
    arz::geometry::Point2D throughPoint{};
    arz::geometry::Point2D endPoint{};
};

struct PastePlacementOverlay final {
    arz::geometry::Point2D basePoint{};
    arz::geometry::Point2D insertionPoint{};
    std::vector<TransientLine> lines;
    std::vector<TransientPolyline> polylines;
    std::vector<TransientCircle> circles;
    std::vector<TransientArc> arcs;
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
    void setDrawingPolyline(TransientPolyline value);
    void clearDrawingPolyline() noexcept;
    [[nodiscard]] const std::optional<TransientPolyline>& drawingPolyline() const noexcept;
    void setDrawingCircle(TransientCircle value);
    void clearDrawingCircle() noexcept;
    [[nodiscard]] const std::optional<TransientCircle>& drawingCircle() const noexcept;
    void setDrawingArc(TransientArc value);
    void clearDrawingArc() noexcept;
    [[nodiscard]] const std::optional<TransientArc>& drawingArc() const noexcept;

private:
    std::string dynamicText_;
    std::optional<SelectionWindowOverlay> selectionWindow_;
    std::vector<std::string> commandSuggestions_;
    std::size_t selectedSuggestionIndex_{0};
    std::optional<PastePlacementOverlay> pastePlacement_;
    std::optional<TransientPolyline> drawingPolyline_;
    std::optional<TransientCircle> drawingCircle_;
    std::optional<TransientArc> drawingArc_;
};

}
