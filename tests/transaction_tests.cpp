#include <cstdlib>
#include <iostream>
#include <memory>

#include "cad/entities/LineEntity.h"
#include "core/commands/ICommand.h"
#include "core/document/Document.h"
#include "core/objects/DocumentObject.h"
#include "core/transactions/TransactionHistory.h"

namespace {

class TestObject final : public arz::core::DocumentObject {
public:
    explicit TestObject(arz::core::ObjectId id)
        : DocumentObject(id) {
    }
};

class AddObjectCommand final : public arz::core::ICommand {
public:
    AddObjectCommand(
        arz::core::Document& document,
        arz::core::ObjectId id
    )
        : document_(document),
          id_(id) {
    }

    bool execute() override {
        return document_.addObject(
            std::make_unique<TestObject>(id_)
        );
    }

    bool undo() override {
        return document_.removeObject(id_);
    }

private:
    arz::core::Document& document_;
    arz::core::ObjectId id_;
};

class DeleteObjectCommand final : public arz::core::ICommand {
public:
    DeleteObjectCommand(
        arz::core::Document& document,
        arz::core::ObjectId id
    )
        : document_(document),
          id_(id) {
    }

    bool execute() override {
        return document_.removeObject(id_);
    }

    bool undo() override {
        return document_.addObject(
            std::make_unique<TestObject>(id_)
        );
    }

private:
    arz::core::Document& document_;
    arz::core::ObjectId id_;
};

class MoveLineEndCommand final : public arz::core::ICommand {
public:
    MoveLineEndCommand(
        arz::core::Document& document,
        arz::core::ObjectId id,
        arz::geometry::Point2D destination
    )
        : document_(document),
          id_(id),
          destination_(destination) {
    }

    bool execute() override {
        auto* line = lineEntity();

        if (line == nullptr) {
            return false;
        }

        if (!hasOrigin_) {
            origin_ = line->end();
            hasOrigin_ = true;
        }

        line->setEnd(destination_);
        return true;
    }

    bool undo() override {
        auto* line = lineEntity();

        if (line == nullptr || !hasOrigin_) {
            return false;
        }

        line->setEnd(origin_);
        return true;
    }

private:
    [[nodiscard]] arz::cad::LineEntity* lineEntity() const {
        return dynamic_cast<arz::cad::LineEntity*>(
            document_.object(id_)
        );
    }

    arz::core::Document& document_;
    arz::core::ObjectId id_;
    arz::geometry::Point2D destination_;
    arz::geometry::Point2D origin_{};
    bool hasOrigin_{false};
};

bool testEmptyHistory() {
    arz::core::TransactionHistory history;

    return !history.canUndo()
        && !history.canRedo()
        && history.undoCount() == 0
        && history.redoCount() == 0
        && !history.execute(nullptr)
        && !history.undo()
        && !history.redo();
}

bool testAddUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!history.execute(
            std::make_unique<AddObjectCommand>(document, id))) {
        return false;
    }

    if (!document.contains(id)
        || history.undoCount() != 1
        || history.redoCount() != 0) {
        return false;
    }

    if (!history.undo()) {
        return false;
    }

    if (document.contains(id)
        || history.canUndo()
        || !history.canRedo()) {
        return false;
    }

    return history.redo()
        && document.contains(id)
        && history.canUndo()
        && !history.canRedo();
}

bool testDeleteUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!document.addObject(
            std::make_unique<TestObject>(id))) {
        return false;
    }

    if (!history.execute(
            std::make_unique<DeleteObjectCommand>(document, id))) {
        return false;
    }

    if (document.contains(id) || !history.undo()) {
        return false;
    }

    if (!document.contains(id) || !history.redo()) {
        return false;
    }

    return !document.contains(id);
}

bool testMoveUndoRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!document.addObject(
            std::make_unique<arz::cad::LineEntity>(
                id,
                arz::cad::DefaultLayerId,
                arz::geometry::Point2D{0.0, 0.0},
                arz::geometry::Point2D{1000.0, 0.0}
            ))) {
        return false;
    }

    if (!history.execute(
            std::make_unique<MoveLineEndCommand>(
                document,
                id,
                arz::geometry::Point2D{2500.0, 500.0}
            ))) {
        return false;
    }

    auto* line = dynamic_cast<arz::cad::LineEntity*>(
        document.object(id)
    );

    if (line == nullptr
        || line->end()
            != arz::geometry::Point2D{2500.0, 500.0}) {
        return false;
    }

    if (!history.undo()
        || line->end()
            != arz::geometry::Point2D{1000.0, 0.0}) {
        return false;
    }

    return history.redo()
        && line->end()
            == arz::geometry::Point2D{2500.0, 500.0};
}

bool testNewExecutionClearsRedo() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto firstId = document.nextObjectId();
    const auto secondId = document.nextObjectId();

    if (!history.execute(
            std::make_unique<AddObjectCommand>(
                document,
                firstId
            ))
        || !history.undo()
        || !history.canRedo()) {
        return false;
    }

    if (!history.execute(
            std::make_unique<AddObjectCommand>(
                document,
                secondId
            ))) {
        return false;
    }

    return !history.canRedo()
        && !history.redo()
        && !document.contains(firstId)
        && document.contains(secondId);
}

bool testFailedExecutionDoesNotChangeHistory() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!history.execute(
            std::make_unique<AddObjectCommand>(document, id))
        || !history.undo()) {
        return false;
    }

    const bool executed = history.execute(
        std::make_unique<DeleteObjectCommand>(document, id)
    );

    return !executed
        && history.undoCount() == 0
        && history.redoCount() == 1
        && history.redo()
        && document.contains(id);
}

bool testFailedUndoRedoPreservesHistory() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!history.execute(
            std::make_unique<AddObjectCommand>(document, id))
        || !document.removeObject(id)) {
        return false;
    }

    if (history.undo()
        || history.undoCount() != 1
        || history.redoCount() != 0) {
        return false;
    }

    if (!document.addObject(std::make_unique<TestObject>(id))
        || !history.undo()) {
        return false;
    }

    if (!document.addObject(std::make_unique<TestObject>(id))) {
        return false;
    }

    if (history.redo()
        || history.undoCount() != 0
        || history.redoCount() != 1) {
        return false;
    }

    return document.removeObject(id)
        && history.redo()
        && history.undoCount() == 1
        && history.redoCount() == 0;
}

bool testClear() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    const auto id = document.nextObjectId();

    if (!history.execute(
            std::make_unique<AddObjectCommand>(document, id))) {
        return false;
    }

    history.clear();

    return document.contains(id)
        && !history.canUndo()
        && !history.canRedo();
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

    run("EmptyHistory", testEmptyHistory());
    run("AddUndoRedo", testAddUndoRedo());
    run("DeleteUndoRedo", testDeleteUndoRedo());
    run("MoveUndoRedo", testMoveUndoRedo());
    run(
        "NewExecutionClearsRedo",
        testNewExecutionClearsRedo()
    );
    run(
        "FailedExecutionDoesNotChangeHistory",
        testFailedExecutionDoesNotChangeHistory()
    );
    run(
        "FailedUndoRedoPreservesHistory",
        testFailedUndoRedoPreservesHistory()
    );
    run("Clear", testClear());

    if (failures != 0) {
        std::cerr
            << failures
            << " transaction test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All transaction tests passed.\n";

    return EXIT_SUCCESS;
}
