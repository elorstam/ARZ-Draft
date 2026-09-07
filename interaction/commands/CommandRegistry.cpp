#include "interaction/commands/CommandRegistry.h"

#include <algorithm>
#include <cctype>

namespace arz::interaction {

namespace {

std::string normalized(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    std::string result(text.substr(first, last - first + 1));
    std::ranges::transform(result, result.begin(), [](unsigned char value) {
        return static_cast<char>(std::toupper(value));
    });
    return result;
}

}

CommandRegistry::CommandRegistry()
    : commands_{
        {CadCommand::Line, "LINE", {"L"}, true},
        {CadCommand::Undo, "UNDO", {}, false},
        {CadCommand::Redo, "REDO", {}, false},
        {CadCommand::Cancel, "CANCEL", {"ESC"}, false}
      } {
}

std::optional<CommandDescriptor> CommandRegistry::resolve(
    std::string_view text
) const {
    const auto candidate = normalized(text);
    for (const auto& descriptor : commands_) {
        if (descriptor.canonicalName == candidate
            || std::ranges::find(descriptor.aliases, candidate)
                != descriptor.aliases.end()) {
            return descriptor;
        }
    }
    return std::nullopt;
}

}
