#include <cstdlib>
#include <iostream>
#include <memory>

#include "cad/commands/AddLineCommand.h"
#include "cad/commands/DeleteEntityCommand.h"
#include "cad/commands/MoveEntityCommand.h"
#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"
#include "core/transactions/TransactionHistory.h"

namespace {

[[nodiscard]] arz::cad::LineEntity* lineEntity(
    arz::core::Document& document,
    arz::core::ObjectId objectId
) {
    return dynamic_cast<arz::cad::LineEntity*>(
        document.object(objectId)
    );
}

bool addLine(
    arz::core::Document& document,
    arz::core::TransactionHistory& history,
    arz::geometry::Point2D start,
    arz::geometry::Point2D end,
    arz::core::ObjectId& objectId
) {
    auto command = std::make_unique<arz::cad::AddLineCommand>(
        document,
        arz::cad::DefaultLayerId,
        start,
        end
    );
    auto* commandView = command.get();

    if (!history.execute(std::move(command))) {
        return false;
    }

    objectId = commandView->objectId();
    return objectId != arz::core::InvalidObjectId;
}

bool testAddLineExecuteUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;

    auto command = std::make_unique<arz::cad::AddLineCommand>(
        document,
        arz::cad::DefaultLayerId,
        arz::geometry::Point2D{100.0, 200.0},
        arz::geometry::Point2D{1100.0, 700.0}
    );
    auto* commandView = command.get();

    if (!history.execute(std::move(command))) {
        return false;
    }

    const auto objectId = commandView->objectId();
    auto* originalLine = lineEntity(document, objectId);

    if (objectId == arz::core::InvalidObjectId
        || originalLine == nullptr
        || originalLine->start()
            != arz::geometry::Point2D{100.0, 200.0}
        || originalLine->end()
            != arz::geometry::Point2D{1100.0, 700.0}) {
        return false;
    }

    if (!history.undo() || document.contains(objectId)) {
        return false;
    }

    if (!history.redo()) {
        return false;
    }

    return lineEntity(document, objectId) == originalLine
        && commandView->objectId() == objectId;
}

bool testInvalidLayerAddFailsSafely() {
    arz::core::Document document;
    arz::core::TransactionHistory history;

    auto command = std::make_unique<arz::cad::AddLineCommand>(
        document,
        arz::cad::InvalidLayerId,
        arz::geometry::Point2D{0.0, 0.0},
        arz::geometry::Point2D{100.0, 0.0}
    );
    const bool executed = history.execute(std::move(command));

    return !executed
        && document.objectCount() == 0
        && !history.canUndo()
        && !history.canRedo();
}

bool testDeleteEntityExecuteUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::core::ObjectId objectId{};

    if (!addLine(
            document,
            history,
            {50.0, 75.0},
            {500.0, 750.0},
            objectId)) {
        return false;
    }

    auto* originalLine = lineEntity(document, objectId);

    if (!history.execute(
            std::make_unique<arz::cad::DeleteEntityCommand>(
                document,
                objectId
            ))) {
        return false;
    }

    if (document.contains(objectId) || !history.undo()) {
        return false;
    }

    auto* restoredLine = lineEntity(document, objectId);

    if (restoredLine != originalLine
        || restoredLine->start()
            != arz::geometry::Point2D{50.0, 75.0}
        || restoredLine->end()
            != arz::geometry::Point2D{500.0, 750.0}) {
        return false;
    }

    return history.redo()
        && !document.contains(objectId);
}

bool testDeleteMissingEntityFailsSafely() {
    arz::core::Document document;
    arz::core::TransactionHistory history;

    const bool invalidExecuted = history.execute(
        std::make_unique<arz::cad::DeleteEntityCommand>(
            document,
            arz::core::InvalidObjectId
        )
    );
    const bool missingExecuted = history.execute(
        std::make_unique<arz::cad::DeleteEntityCommand>(
            document,
            500
        )
    );

    return !invalidExecuted
        && !missingExecuted
        && document.objectCount() == 0
        && !history.canUndo()
        && !history.canRedo();
}

bool testMoveLineExecuteUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::core::ObjectId objectId{};

    if (!addLine(
            document,
            history,
            {10.25, 20.5},
            {1010.75, 2020.25},
            objectId)) {
        return false;
    }

    if (!history.execute(
            std::make_unique<arz::cad::MoveEntityCommand>(
                document,
                objectId,
                arz::geometry::Point2D{125.5, -75.25}
            ))) {
        return false;
    }

    auto* line = lineEntity(document, objectId);

    if (line == nullptr
        || line->start()
            != arz::geometry::Point2D{135.75, -54.75}
        || line->end()
            != arz::geometry::Point2D{1136.25, 1945.0}) {
        return false;
    }

    if (!history.undo()
        || line->start()
            != arz::geometry::Point2D{10.25, 20.5}
        || line->end()
            != arz::geometry::Point2D{1010.75, 2020.25}) {
        return false;
    }

    if (!history.redo()
        || line->start()
            != arz::geometry::Point2D{135.75, -54.75}
        || line->end()
            != arz::geometry::Point2D{1136.25, 1945.0}) {
        return false;
    }

    for (int cycle = 0; cycle < 20; ++cycle) {
        if (!history.undo() || !history.redo()) {
            return false;
        }
    }

    return line->start()
            == arz::geometry::Point2D{135.75, -54.75}
        && line->end()
            == arz::geometry::Point2D{1136.25, 1945.0};
}

bool testSequentialCommandsUndoRedoOrder() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::core::ObjectId firstId{};
    arz::core::ObjectId secondId{};

    if (!addLine(
            document,
            history,
            {0.0, 0.0},
            {100.0, 0.0},
            firstId)
        || !addLine(
            document,
            history,
            {0.0, 100.0},
            {100.0, 100.0},
            secondId)
        || !history.execute(
            std::make_unique<arz::cad::MoveEntityCommand>(
                document,
                firstId,
                arz::geometry::Point2D{25.0, 50.0}
            ))) {
        return false;
    }

    auto* firstLine = lineEntity(document, firstId);

    if (firstLine == nullptr
        || firstLine->start()
            != arz::geometry::Point2D{25.0, 50.0}) {
        return false;
    }

    if (!history.undo()
        || firstLine->start()
            != arz::geometry::Point2D{0.0, 0.0}
        || !document.contains(secondId)) {
        return false;
    }

    if (!history.undo()
        || document.contains(secondId)
        || !document.contains(firstId)) {
        return false;
    }

    if (!history.undo()
        || document.contains(firstId)
        || history.undoCount() != 0) {
        return false;
    }

    if (!history.redo()
        || !document.contains(firstId)
        || document.contains(secondId)) {
        return false;
    }

    if (!history.redo()
        || !document.contains(secondId)) {
        return false;
    }

    if (!history.redo()) {
        return false;
    }

    firstLine = lineEntity(document, firstId);

    return firstLine != nullptr
        && firstLine->start()
            == arz::geometry::Point2D{25.0, 50.0}
        && history.undoCount() == 3
        && history.redoCount() == 0;
}

bool testNewCommandClearsRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::core::ObjectId firstId{};
    arz::core::ObjectId secondId{};

    if (!addLine(
            document,
            history,
            {0.0, 0.0},
            {100.0, 0.0},
            firstId)
        || !addLine(
            document,
            history,
            {0.0, 100.0},
            {100.0, 100.0},
            secondId)
        || !history.undo()
        || !history.canRedo()) {
        return false;
    }

    if (!history.execute(
            std::make_unique<arz::cad::MoveEntityCommand>(
                document,
                firstId,
                arz::geometry::Point2D{10.0, 20.0}
            ))) {
        return false;
    }

    return !history.canRedo()
        && !history.redo()
        && !document.contains(secondId)
        && lineEntity(document, firstId)->start()
            == arz::geometry::Point2D{10.0, 20.0};
}

bool testFailedCommandPreservesHistory() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::core::ObjectId firstId{};
    arz::core::ObjectId secondId{};

    if (!addLine(
            document,
            history,
            {0.0, 0.0},
            {100.0, 0.0},
            firstId)
        || !addLine(
            document,
            history,
            {0.0, 100.0},
            {100.0, 100.0},
            secondId)
        || !history.undo()) {
        return false;
    }

    const auto undoCount = history.undoCount();
    const auto redoCount = history.redoCount();

    const bool executed = history.execute(
        std::make_unique<arz::cad::DeleteEntityCommand>(
            document,
            999999
        )
    );

    if (executed
        || history.undoCount() != undoCount
        || history.redoCount() != redoCount
        || document.contains(secondId)) {
        return false;
    }

    return history.redo()
        && document.contains(firstId)
        && document.contains(secondId);
}

}

int main() {
    int failures = 0;

    const auto run = [&failures](
        const char* name,
        bool result
    ) {
        if (result) {
            std::cout
                << "[PASS] "
                << name
                << '\n';
        } else {
            std::cerr
                << "[FAIL] "
                << name
                << '\n';

            ++failures;
        }
    };

    run(
        "AddLineExecuteUndoRedo",
        testAddLineExecuteUndoRedo()
    );
    run(
        "InvalidLayerAddFailsSafely",
        testInvalidLayerAddFailsSafely()
    );
    run(
        "DeleteEntityExecuteUndoRedo",
        testDeleteEntityExecuteUndoRedo()
    );
    run(
        "DeleteMissingEntityFailsSafely",
        testDeleteMissingEntityFailsSafely()
    );
    run(
        "MoveLineExecuteUndoRedo",
        testMoveLineExecuteUndoRedo()
    );
    run(
        "SequentialCommandsUndoRedoOrder",
        testSequentialCommandsUndoRedoOrder()
    );
    run(
        "NewCommandClearsRedo",
        testNewCommandClearsRedo()
    );
    run(
        "FailedCommandPreservesHistory",
        testFailedCommandPreservesHistory()
    );

    if (failures != 0) {
        std::cerr
            << failures
            << " command test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All command tests passed.\n";

    return EXIT_SUCCESS;
}
