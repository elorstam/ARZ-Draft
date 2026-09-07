#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include "cad/commands/AddLineCommand.h"
#include "cad/commands/DeleteEntityCommand.h"
#include "cad/entities/LineEntity.h"
#include "cad/selection/LineEntityPickRefiner.h"
#include "cad/selection/SelectionService.h"
#include "cad/spatial/LinearSpatialIndex2D.h"
#include "core/document/Document.h"
#include "core/transactions/TransactionHistory.h"

namespace {

bool addIndexedLine(
    arz::core::Document& document,
    arz::core::TransactionHistory& history,
    arz::cad::LinearSpatialIndex2D& index,
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
    const auto* line = dynamic_cast<const arz::cad::LineEntity*>(
        document.object(objectId)
    );

    return line != nullptr
        && index.insert(objectId, line->boundingBox());
}

bool testIndexLifecycle() {
    arz::cad::LinearSpatialIndex2D index;
    const arz::geometry::BoundingBox2D originQuery{
        -10.0,
        -10.0,
        10.0,
        10.0
    };

    if (!index.query(originQuery).empty()
        || index.size() != 0
        || index.insert(
            arz::core::InvalidObjectId,
            originQuery
        )) {
        return false;
    }

    if (!index.insert(20, {0.0, 0.0, 100.0, 100.0})
        || index.insert(20, {200.0, 200.0, 300.0, 300.0})
        || !index.insert(10, {50.0, 50.0, 150.0, 150.0})) {
        return false;
    }

    const auto initial = index.query({75.0, 75.0, 80.0, 80.0});

    if (initial != std::vector<arz::core::ObjectId>{10, 20}
        || index.size() != 2) {
        return false;
    }

    if (!index.update(20, {400.0, 400.0, 300.0, 300.0})
        || index.update(99, originQuery)
        || index.update(arz::core::InvalidObjectId, originQuery)) {
        return false;
    }

    if (!index.query(originQuery).empty()
        || index.query({350.0, 350.0, 351.0, 351.0})
            != std::vector<arz::core::ObjectId>{20}) {
        return false;
    }

    return index.remove(20)
        && !index.remove(20)
        && !index.remove(arz::core::InvalidObjectId)
        && index.size() == 1
        && index.query({300.0, 300.0, 400.0, 400.0}).empty();
}

bool testBoundingBoxRelations() {
    const arz::geometry::BoundingBox2D reversed{
        100.0,
        80.0,
        0.0,
        -20.0
    };
    const auto normalized = reversed.normalized();

    return normalized.minX == 0.0
        && normalized.minY == -20.0
        && normalized.maxX == 100.0
        && normalized.maxY == 80.0
        && reversed.intersects({90.0, 70.0, 110.0, 90.0})
        && !reversed.intersects({101.0, 81.0, 120.0, 100.0})
        && reversed.contains({20.0, 20.0, 10.0, 10.0})
        && !reversed.contains({-1.0, 0.0, 10.0, 10.0});
}

bool testWindowSelectionAndReversedCoordinates() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntityPickRefiner refiner;
    arz::cad::SelectionService selection(
        document,
        index,
        refiner
    );
    arz::core::ObjectId insideId{};
    arz::core::ObjectId crossingId{};
    arz::core::ObjectId outsideId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {10.0, 10.0},
            {20.0, 20.0},
            insideId)
        || !addIndexedLine(
            document,
            history,
            index,
            {-50.0, 50.0},
            {50.0, 50.0},
            crossingId)
        || !addIndexedLine(
            document,
            history,
            index,
            {200.0, 200.0},
            {300.0, 300.0},
            outsideId)) {
        return false;
    }

    const auto crossing = selection.crossingWindow(
        {100.0, 100.0},
        {0.0, 0.0}
    );
    const auto contained = selection.containedWindow(
        {100.0, 100.0},
        {0.0, 0.0}
    );

    return crossing
            == std::vector<arz::core::ObjectId>{
                insideId,
                crossingId
            }
        && contained
            == std::vector<arz::core::ObjectId>{insideId}
        && crossingId != insideId
        && outsideId != crossingId;
}

bool testPointPickRefinementAndTolerance() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntityPickRefiner refiner;
    arz::cad::SelectionService selection(
        document,
        index,
        refiner
    );
    arz::core::ObjectId diagonalId{};
    arz::core::ObjectId firstId{};
    arz::core::ObjectId secondId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {0.0, 0.0},
            {100.0, 100.0},
            diagonalId)
        || !addIndexedLine(
            document,
            history,
            index,
            {0.0, 0.0},
            {100.0, 0.0},
            firstId)
        || !addIndexedLine(
            document,
            history,
            index,
            {0.0, 3.0},
            {100.0, 3.0},
            secondId)) {
        return false;
    }

    const auto nearLines = selection.pointPick({50.0, 1.0}, 2.0);

    if (nearLines
        != std::vector<arz::core::ObjectId>{firstId, secondId}) {
        return false;
    }

    return selection.pointPick({50.0, 10.0}, 2.0).empty()
        && selection.pointPick({0.0, 100.0}, 1.0).empty()
        && selection.pointPick({50.0, 1.0}, -1.0).empty()
        && diagonalId != firstId;
}

bool testRemovedEntityIsNotSelectable() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntityPickRefiner refiner;
    arz::cad::SelectionService selection(
        document,
        index,
        refiner
    );
    arz::core::ObjectId objectId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {0.0, 0.0},
            {100.0, 0.0},
            objectId)) {
        return false;
    }

    if (selection.pointPick({50.0, 1.0}, 2.0)
        != std::vector<arz::core::ObjectId>{objectId}) {
        return false;
    }

    if (!history.execute(
            std::make_unique<arz::cad::DeleteEntityCommand>(
                document,
                objectId
            ))) {
        return false;
    }

    if (!selection.pointPick({50.0, 1.0}, 2.0).empty()
        || !index.remove(objectId)) {
        return false;
    }

    return selection.pointPick({50.0, 1.0}, 2.0).empty()
        && selection.crossingWindow(
            {-10.0, -10.0},
            {110.0, 10.0}
        ).empty();
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

    run("IndexLifecycle", testIndexLifecycle());
    run("BoundingBoxRelations", testBoundingBoxRelations());
    run(
        "WindowSelectionAndReversedCoordinates",
        testWindowSelectionAndReversedCoordinates()
    );
    run(
        "PointPickRefinementAndTolerance",
        testPointPickRefinementAndTolerance()
    );
    run(
        "RemovedEntityIsNotSelectable",
        testRemovedEntityIsNotSelectable()
    );

    if (failures != 0) {
        std::cerr
            << failures
            << " selection test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All selection tests passed.\n";

    return EXIT_SUCCESS;
}
