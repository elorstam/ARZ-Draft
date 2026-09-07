#include "interaction/commands/CommandRegistry.h"

#include <algorithm>
#include <cctype>
#include <utility>

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
        {CadCommand::Polyline, "PLINE", {"PL"}, true},
        {CadCommand::Circle, "CIRCLE", {"C"}, true},
        {CadCommand::Arc, "ARC", {"A"}, true},
        {CadCommand::Copy, "COPY", {"CO"}, true},
        {CadCommand::Undo, "UNDO", {}, false},
        {CadCommand::Redo, "REDO", {}, false},
        {CadCommand::Cancel, "CANCEL", {"ESC"}, false}
      } {
}

CommandRegistry::CommandRegistry(std::vector<CommandDescriptor> commands)
    : commands_(std::move(commands)) {
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

std::vector<CommandDescriptor> CommandRegistry::suggest(
    std::string_view prefix
) const {
    const auto candidate = normalized(prefix);
    if (candidate.empty()) return {};
    struct Match final { CommandDescriptor descriptor; int rank{}; };
    std::vector<Match> matches;
    for (const auto& descriptor : commands_) {
        int rank = 3;
        if (descriptor.canonicalName == candidate) rank = 0;
        else if (descriptor.canonicalName.starts_with(candidate)) rank = 1;
        for (const auto& alias : descriptor.aliases) {
            if (alias == candidate) rank = std::min(rank, 0);
            else if (alias.starts_with(candidate)) rank = std::min(rank, 2);
        }
        if (rank < 3) matches.push_back({descriptor, rank});
    }
    std::ranges::sort(matches, [](const Match& left, const Match& right) {
        if (left.rank != right.rank) return left.rank < right.rank;
        return left.descriptor.canonicalName < right.descriptor.canonicalName;
    });
    std::vector<CommandDescriptor> result;
    for (auto& match : matches) result.push_back(std::move(match.descriptor));
    return result;
}

}
