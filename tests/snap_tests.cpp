#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "cad/commands/AddLineCommand.h"
#include "cad/entities/LineEntity.h"
#include "cad/snapping/LineEntitySnapProvider.h"
#include "cad/snapping/SnapService.h"
#include "cad/spatial/LinearSpatialIndex2D.h"
#include "core/document/Document.h"
#include "core/transactions/TransactionHistory.h"

namespace {

constexpr std::array AllSnapTypes{
    arz::cad::SnapType::Endpoint,
    arz::cad::SnapType::Midpoint
};

constexpr std::array EndpointOnly{
    arz::cad::SnapType::Endpoint
};

constexpr std::array MidpointOnly{
    arz::cad::SnapType::Midpoint
};

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

bool testEndpointSnapAtLineEnds() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId objectId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {10.0, 20.0},
            {110.0, 20.0},
            objectId)) {
        return false;
    }

    const auto start = snaps.bestSnap(
        {12.0, 20.0},
        5.0,
        EndpointOnly
    );
    const auto end = snaps.bestSnap(
        {109.0, 20.0},
        5.0,
        EndpointOnly
    );

    return start.has_value()
        && start->objectId == objectId
        && start->type == arz::cad::SnapType::Endpoint
        && start->point == arz::geometry::Point2D{10.0, 20.0}
        && start->distance == 2.0
        && end.has_value()
        && end->objectId == objectId
        && end->point == arz::geometry::Point2D{110.0, 20.0}
        && end->distance == 1.0;
}

bool testMidpointAndReversedLine() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId objectId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {100.0, 40.0},
            {0.0, 20.0},
            objectId)) {
        return false;
    }

    const auto result = snaps.bestSnap(
        {51.0, 30.0},
        5.0,
        MidpointOnly
    );

    return result.has_value()
        && result->objectId == objectId
        && result->type == arz::cad::SnapType::Midpoint
        && result->point == arz::geometry::Point2D{50.0, 30.0}
        && result->distance == 1.0;
}

bool testOutsideToleranceAndBoundsOnlyRejected() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId objectId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {0.0, 0.0},
            {100.0, 100.0},
            objectId)) {
        return false;
    }

    return !snaps.bestSnap(
            {0.0, 100.0},
            2.0,
            AllSnapTypes
        ).has_value()
        && !snaps.bestSnap(
            {10.0, 0.0},
            5.0,
            EndpointOnly
        ).has_value();
}

bool testNearestRankingEndpointVersusMidpoint() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
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

    const auto result = snaps.bestSnap(
        {45.0, 0.0},
        60.0,
        AllSnapTypes
    );
    const auto tied = snaps.bestSnap(
        {25.0, 0.0},
        30.0,
        AllSnapTypes
    );

    return result.has_value()
        && result->objectId == objectId
        && result->type == arz::cad::SnapType::Midpoint
        && result->point == arz::geometry::Point2D{50.0, 0.0}
        && result->distance == 5.0
        && tied.has_value()
        && tied->type == arz::cad::SnapType::Endpoint
        && tied->point == arz::geometry::Point2D{0.0, 0.0}
        && tied->distance == 25.0;
}

bool testMultipleEntitiesAndDeterministicTie() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId firstId{};
    arz::core::ObjectId secondId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {-10.0, 0.0},
            {-20.0, 0.0},
            firstId)
        || !addIndexedLine(
            document,
            history,
            index,
            {10.0, 0.0},
            {20.0, 0.0},
            secondId)) {
        return false;
    }

    const auto candidates = snaps.candidates(
        {0.0, 0.0},
        10.0,
        EndpointOnly
    );
    const auto best = snaps.bestSnap(
        {0.0, 0.0},
        10.0,
        EndpointOnly
    );

    return candidates.size() == 2
        && candidates[0].objectId == firstId
        && candidates[1].objectId == secondId
        && best.has_value()
        && best->objectId == firstId
        && firstId < secondId;
}

bool testEnabledAndDisabledModes() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
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

    const auto endpoint = snaps.bestSnap(
        {50.0, 0.0},
        60.0,
        EndpointOnly
    );
    const auto midpoint = snaps.bestSnap(
        {50.0, 0.0},
        60.0,
        MidpointOnly
    );
    const std::array<arz::cad::SnapType, 0> disabled{};

    return endpoint.has_value()
        && endpoint->type == arz::cad::SnapType::Endpoint
        && endpoint->point == arz::geometry::Point2D{0.0, 0.0}
        && midpoint.has_value()
        && midpoint->type == arz::cad::SnapType::Midpoint
        && midpoint->point == arz::geometry::Point2D{50.0, 0.0}
        && !snaps.bestSnap(
            {50.0, 0.0},
            60.0,
            disabled
        ).has_value();
}

bool testStaleAndRemovedSpatialObjects() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId objectId{};

    if (!index.insert(999, {-1.0, -1.0, 1.0, 1.0})
        || snaps.bestSnap(
            {0.0, 0.0},
            2.0,
            AllSnapTypes
        ).has_value()
        || !addIndexedLine(
            document,
            history,
            index,
            {10.0, 10.0},
            {20.0, 10.0},
            objectId)) {
        return false;
    }

    if (!snaps.bestSnap(
            {10.0, 10.0},
            1.0,
            EndpointOnly
        ).has_value()
        || !index.remove(objectId)) {
        return false;
    }

    return !snaps.bestSnap(
        {10.0, 10.0},
        1.0,
        EndpointOnly
    ).has_value();
}

bool testZeroLengthLineAndNoDuplicates() {
    arz::core::Document document;
    arz::core::TransactionHistory history;
    arz::cad::LinearSpatialIndex2D index;
    arz::cad::LineEntitySnapProvider provider;
    arz::cad::SnapService snaps(document, index, provider);
    arz::core::ObjectId objectId{};

    if (!addIndexedLine(
            document,
            history,
            index,
            {25.0, 25.0},
            {25.0, 25.0},
            objectId)) {
        return false;
    }

    constexpr std::array RepeatedTypes{
        arz::cad::SnapType::Endpoint,
        arz::cad::SnapType::Endpoint,
        arz::cad::SnapType::Midpoint,
        arz::cad::SnapType::Midpoint
    };
    const auto candidates = snaps.candidates(
        {25.0, 25.0},
        0.0,
        RepeatedTypes
    );
    const auto midpoint = snaps.candidates(
        {25.0, 25.0},
        0.0,
        MidpointOnly
    );

    return candidates.size() == 1
        && candidates.front().objectId == objectId
        && candidates.front().type == arz::cad::SnapType::Endpoint
        && candidates.front().point
            == arz::geometry::Point2D{25.0, 25.0}
        && candidates.front().distance == 0.0
        && midpoint.size() == 1
        && midpoint.front().type == arz::cad::SnapType::Midpoint;
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

    run("EndpointSnapAtLineEnds", testEndpointSnapAtLineEnds());
    run("MidpointAndReversedLine", testMidpointAndReversedLine());
    run(
        "OutsideToleranceAndBoundsOnlyRejected",
        testOutsideToleranceAndBoundsOnlyRejected()
    );
    run(
        "NearestRankingEndpointVersusMidpoint",
        testNearestRankingEndpointVersusMidpoint()
    );
    run(
        "MultipleEntitiesAndDeterministicTie",
        testMultipleEntitiesAndDeterministicTie()
    );
    run(
        "EnabledAndDisabledModes",
        testEnabledAndDisabledModes()
    );
    run(
        "StaleAndRemovedSpatialObjects",
        testStaleAndRemovedSpatialObjects()
    );
    run(
        "ZeroLengthLineAndNoDuplicates",
        testZeroLengthLineAndNoDuplicates()
    );

    if (failures != 0) {
        std::cerr
            << failures
            << " snap test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All snap tests passed.\n";

    return EXIT_SUCCESS;
}
