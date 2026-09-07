#include "interaction/commands/CommandLineParser.h"

#include "interaction/commands/CommandRegistry.h"

namespace arz::interaction {

EditorCommand CommandLineParser::parse(std::string_view text) {
    const auto descriptor = CommandRegistry{}.resolve(text);
    if (!descriptor) return EditorCommand::Unknown;
    switch (descriptor->command) {
    case CadCommand::Line: return EditorCommand::Line;
    case CadCommand::Polyline: return EditorCommand::Polyline;
    case CadCommand::Circle: return EditorCommand::Circle;
    case CadCommand::Arc: return EditorCommand::Arc;
    case CadCommand::Undo: return EditorCommand::Undo;
    case CadCommand::Redo: return EditorCommand::Redo;
    case CadCommand::Cancel: return EditorCommand::Cancel;
    case CadCommand::Unknown: return EditorCommand::Unknown;
    }
    return EditorCommand::Unknown;
}

}
