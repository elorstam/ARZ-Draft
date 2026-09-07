#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace arz::interaction {

class CommandInputState final {
public:
    void append(char character);
    void backspace() noexcept;
    void setBuffer(std::string text);
    void clearBuffer() noexcept;
    [[nodiscard]] const std::string& buffer() const noexcept;

    void recordInvocation(std::string canonicalName, bool repeatable);
    [[nodiscard]] const std::vector<std::string>& history() const noexcept;
    [[nodiscard]] const std::optional<std::string>& lastRepeatable() const noexcept;

private:
    std::string buffer_;
    std::vector<std::string> history_;
    std::optional<std::string> lastRepeatable_;
};

}
