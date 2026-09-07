#pragma once

#include <memory>

#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"

namespace arz::core {
class Document;
}

namespace arz::cad {

class DeleteEntityCommand final : public arz::core::ICommand {
public:
    DeleteEntityCommand(
        arz::core::Document& document,
        arz::core::ObjectId objectId
    );

    bool execute() override;
    bool undo() override;

private:
    arz::core::Document& document_;
    arz::core::ObjectId objectId_;
    std::unique_ptr<arz::core::DocumentObject> entity_;
    arz::core::DocumentObject* identity_{nullptr};
    bool deleted_{false};
};

}
