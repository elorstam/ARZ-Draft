#include "core/transactions/TransactionHistory.h"

#include <utility>

namespace arz::core {

bool TransactionHistory::execute(
    std::unique_ptr<ICommand> command
) {
    if (!command) {
        return false;
    }

    undoStack_.reserve(undoStack_.size() + 1);

    if (!command->execute()) {
        return false;
    }

    undoStack_.push_back(std::move(command));
    redoStack_.clear();
    return true;
}

bool TransactionHistory::undo() {
    if (undoStack_.empty()) {
        return false;
    }

    redoStack_.reserve(redoStack_.size() + 1);
    auto& command = undoStack_.back();

    if (!command->undo()) {
        return false;
    }

    redoStack_.push_back(std::move(command));
    undoStack_.pop_back();
    return true;
}

bool TransactionHistory::redo() {
    if (redoStack_.empty()) {
        return false;
    }

    undoStack_.reserve(undoStack_.size() + 1);
    auto& command = redoStack_.back();

    if (!command->redo()) {
        return false;
    }

    undoStack_.push_back(std::move(command));
    redoStack_.pop_back();
    return true;
}

bool TransactionHistory::canUndo() const noexcept {
    return !undoStack_.empty();
}

bool TransactionHistory::canRedo() const noexcept {
    return !redoStack_.empty();
}

std::size_t TransactionHistory::undoCount() const noexcept {
    return undoStack_.size();
}

std::size_t TransactionHistory::redoCount() const noexcept {
    return redoStack_.size();
}

void TransactionHistory::clear() noexcept {
    undoStack_.clear();
    redoStack_.clear();
}

}
