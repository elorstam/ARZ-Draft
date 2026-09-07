#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "cad/layers/LayerId.h"
#include "cad/selection/CadEntityPickRefiner.h"
#include "cad/selection/SelectionService.h"
#include "cad/snapping/CadEntitySnapProvider.h"
#include "cad/snapping/SnapService.h"
#include "cad/spatial/LinearSpatialIndex2D.h"
#include "core/document/Document.h"
#include "core/transactions/TransactionHistory.h"
#include "interaction/clipboard/CadClipboard.h"
#include "interaction/commands/CommandInputState.h"
#include "interaction/commands/CommandRegistry.h"
#include "interaction/drafting/DraftingSettings.h"
#include "interaction/overlays/OverlayState.h"
#include "interaction/selection/SelectionSet.h"
#include "interaction/tools/LineInputController.h"
#include "interaction/tools/PolylineInputController.h"
#include "interaction/tools/CircleInputController.h"
#include "interaction/tools/ArcInputController.h"

namespace arz::app {

enum class CanvasAction { None = 0, FirstLinePointAccepted, EntityCreated, SelectionChanged, SelectionWindowStarted };

class CadApplicationController final {
public:
    CadApplicationController();
    void startLine() noexcept;
    void startPolyline() noexcept;
    void startCircle() noexcept;
    void startArc() noexcept;
    void cancel() noexcept;
    void appendCommandCharacter(char character);
    void backspaceCommandBuffer() noexcept;
    void setCommandBuffer(std::string text);
    [[nodiscard]] bool confirmInput();
    [[nodiscard]] bool escape();
    [[nodiscard]] bool invokeCommandText(std::string_view text);
    [[nodiscard]] bool rightClick();
    void selectPreviousSuggestion() noexcept;
    void selectNextSuggestion() noexcept;
    void updatePointer(arz::geometry::Point2D worldPoint,
                       double worldTolerance = 0.0);

    [[nodiscard]] CanvasAction canvasClick(arz::geometry::Point2D worldPoint,
                                           double worldTolerance,
                                           bool shiftModifier = false);
    void beginSelectionWindow(arz::geometry::Point2D first);
    void updateSelectionWindow(arz::geometry::Point2D current);
    [[nodiscard]] bool finishSelectionWindow(bool removalMode);
    [[nodiscard]] std::optional<arz::cad::SnapResult> snapCandidate(
        arz::geometry::Point2D worldPoint, double worldTolerance) const;

    bool undo();
    bool redo();
    bool deleteSelection();
    bool copySelection();
    bool cutSelection();
    bool paste();
    [[nodiscard]] bool pastePlacementActive() const noexcept;
    bool selectAll();

    [[nodiscard]] const arz::core::Document& document() const noexcept;
    [[nodiscard]] const arz::cad::ISpatialIndex2D& spatialIndex() const noexcept;
    [[nodiscard]] const arz::core::TransactionHistory& history() const noexcept;
    [[nodiscard]] const arz::interaction::SelectionSet& selection() const noexcept;
    [[nodiscard]] arz::core::ObjectId selectedObjectId() const noexcept;
    [[nodiscard]] const arz::interaction::CadClipboard& clipboard() const noexcept;
    [[nodiscard]] const arz::interaction::DraftingSettings& draftingSettings() const noexcept;
    [[nodiscard]] const arz::interaction::OverlayState& overlayState() const noexcept;
    [[nodiscard]] const arz::interaction::CommandInputState& commandInput() const noexcept;
    bool toggleDrafting(arz::interaction::DraftingToggle toggle) noexcept;

    [[nodiscard]] arz::cad::LayerId currentLayerId() const noexcept;
    bool setCurrentLayerId(arz::cad::LayerId layerId) noexcept;
    [[nodiscard]] arz::interaction::LineInputState lineInputState() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> lineStartPoint() const noexcept;
    [[nodiscard]] arz::interaction::PolylineInputState polylineInputState() const noexcept;
    [[nodiscard]] const std::vector<arz::geometry::Point2D>& polylineVertices() const noexcept;
    [[nodiscard]] arz::interaction::CircleInputState circleInputState() const noexcept;
    [[nodiscard]] std::optional<arz::geometry::Point2D> circleCenter() const noexcept;
    [[nodiscard]] arz::interaction::ArcInputState arcInputState() const noexcept;
    [[nodiscard]] std::string commandPrompt() const;

private:
    [[nodiscard]] bool invoke(const arz::interaction::CommandDescriptor& descriptor);
    [[nodiscard]] bool finishActiveInteraction();
    [[nodiscard]] bool rebuildSpatialIndex();
    void synchronizeAfterModelChange();
    void updateOverlayText();
    [[nodiscard]] bool commitPaste(arz::geometry::Point2D insertionPoint);
    [[nodiscard]] bool commitPolyline(bool closed);
    [[nodiscard]] bool anyDrawingCommandActive() const noexcept;
    void cancelDrawingCommands() noexcept;

    arz::core::Document document_;
    arz::core::TransactionHistory history_;
    arz::cad::LinearSpatialIndex2D spatialIndex_;
    arz::cad::CadEntityPickRefiner pickRefiner_;
    arz::cad::SelectionService selectionService_;
    arz::cad::CadEntitySnapProvider snapProvider_;
    arz::cad::SnapService snapService_;
    arz::interaction::LineInputController lineInput_;
    arz::interaction::PolylineInputController polylineInput_;
    arz::interaction::CircleInputController circleInput_;
    arz::interaction::ArcInputController arcInput_;
    arz::interaction::CommandRegistry commandRegistry_;
    arz::interaction::CommandInputState commandInput_;
    arz::interaction::SelectionSet selection_;
    arz::interaction::CadClipboard clipboard_;
    arz::interaction::DraftingSettings draftingSettings_;
    arz::interaction::OverlayState overlayState_;
    arz::cad::LayerId currentLayerId_{arz::cad::DefaultLayerId};
    std::string activeOptionBuffer_;
};

}
