#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "core/commands/ICommand.h"

namespace arz::core {

class TransactionHistory final {
public:
    bool execute(std::unique_ptr<ICommand> command);
    bool undo();
    bool redo();

    [[nodiscard]] bool canUndo() const noexcept;
    [[nodiscard]] bool canRedo() const noexcept;

    [[nodiscard]] std::size_t undoCount() const noexcept;
    [[nodiscard]] std::size_t redoCount() const noexcept;

    void clear() noexcept;

private:
    std::vector<std::unique_ptr<ICommand>> undoStack_;
    std::vector<std::unique_ptr<ICommand>> redoStack_;
};

}
