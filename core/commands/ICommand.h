#pragma once

namespace arz::core {

class ICommand {
public:
    virtual ~ICommand() = default;

    // Operations are atomic. A false result leaves the model unchanged.
    virtual bool execute() = 0;
    virtual bool undo() = 0;

    virtual bool redo() {
        return execute();
    }
};

}
