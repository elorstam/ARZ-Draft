#pragma once

namespace arz::interaction {

enum class InteractionStage {
    Idle = 0,
    SelectionActive,
    AwaitingSelectionConfirmation,
    AwaitingPoint,
    AwaitingRepeatingDestination,
    ContextMenuOpen
};

enum class RightClickAction {
    ShowContextMenu = 0,
    ConfirmSelection,
    FinishInteraction,
    NoAction
};

class CommandInputPolicy final {
public:
    [[nodiscard]] static constexpr RightClickAction rightClick(
        InteractionStage stage
    ) noexcept {
        switch (stage) {
        case InteractionStage::AwaitingSelectionConfirmation:
            return RightClickAction::ConfirmSelection;
        case InteractionStage::AwaitingPoint:
        case InteractionStage::AwaitingRepeatingDestination:
            return RightClickAction::FinishInteraction;
        case InteractionStage::Idle:
        case InteractionStage::SelectionActive:
            return RightClickAction::ShowContextMenu;
        case InteractionStage::ContextMenuOpen:
            return RightClickAction::NoAction;
        }
        return RightClickAction::NoAction;
    }
};

}
