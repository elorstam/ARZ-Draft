#include "interaction/commands/CommandInputState.h"

#include <cctype>
#include <utility>

namespace arz::interaction {

void CommandInputState::append(char character) {
    const auto value = static_cast<unsigned char>(character);
    if (std::isprint(value)) {
        buffer_.push_back(static_cast<char>(std::toupper(value)));
    }
}

void CommandInputState::backspace() noexcept {
    if (!buffer_.empty()) {
        buffer_.pop_back();
    }
}

void CommandInputState::setBuffer(std::string text) {
    buffer_.clear();
    for (const char character : text) {
        append(character);
    }
}

void CommandInputState::clearBuffer() noexcept {
    buffer_.clear();
}

const std::string& CommandInputState::buffer() const noexcept {
    return buffer_;
}

void CommandInputState::recordInvocation(
    std::string canonicalName,
    bool repeatable
) {
    history_.push_back(canonicalName);
    if (repeatable) {
        lastRepeatable_ = std::move(canonicalName);
    }
}

const std::vector<std::string>& CommandInputState::history() const noexcept {
    return history_;
}

const std::optional<std::string>&
CommandInputState::lastRepeatable() const noexcept {
    return lastRepeatable_;
}

}
