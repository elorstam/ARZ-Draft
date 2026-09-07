#pragma once

#include <string_view>

namespace arz::interaction {

enum class EditorCommand {
    Unknown = 0,
    Line,
    Undo,
    Redo,
    Cancel
};

class CommandLineParser final {
public:
    [[nodiscard]] static EditorCommand parse(std::string_view text);
};

}
