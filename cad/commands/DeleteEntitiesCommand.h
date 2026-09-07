#pragma once

#include <memory>
#include <vector>

#include "core/commands/ICommand.h"
#include "core/objects/DocumentObject.h"

namespace arz::core { class Document; }

namespace arz::cad {

class DeleteEntitiesCommand final : public arz::core::ICommand {
public:
    DeleteEntitiesCommand(
        arz::core::Document& document,
        std::vector<arz::core::ObjectId> objectIds
    );

    bool execute() override;
    bool undo() override;

private:
    arz::core::Document& document_;
    std::vector<arz::core::ObjectId> objectIds_;
    std::vector<std::unique_ptr<arz::core::DocumentObject>> entities_;
    std::vector<arz::core::DocumentObject*> identities_;
    bool deleted_{false};
};

}
