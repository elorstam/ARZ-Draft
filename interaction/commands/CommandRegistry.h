#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace arz::interaction {

enum class CadCommand {
    Unknown = 0,
    Line,
    Undo,
    Redo,
    Cancel
};

struct CommandDescriptor final {
    CadCommand command{CadCommand::Unknown};
    std::string canonicalName;
    std::vector<std::string> aliases;
    bool repeatable{false};
};

class CommandRegistry final {
public:
    CommandRegistry();

    [[nodiscard]] std::optional<CommandDescriptor> resolve(
        std::string_view text
    ) const;

private:
    std::vector<CommandDescriptor> commands_;
};

}
