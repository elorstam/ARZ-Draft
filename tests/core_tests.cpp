#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "cad/entities/LineEntity.h"
#include "cad/layers/LayerId.h"
#include "core/document/Document.h"
#include "core/objects/DocumentObject.h"
#include "core/objects/ObjectId.h"
#include "core/units/Length.h"
#include "geometry/primitives/BoundingBox2D.h"
#include "geometry/primitives/Point2D.h"
#include "geometry/tolerance/GeometryTolerance.h"

namespace {

class TestObject final : public arz::core::DocumentObject {
public:
    explicit TestObject(arz::core::ObjectId id)
        : DocumentObject(id) {
    }
};

bool nearlyEqual(double a, double b) {
    return std::abs(a - b) <= 1e-9;
}

bool testLength() {
    const auto value = arz::core::Length::fromMeters(3.2);

    return value.millimeters() == 3200.0
        && value.centimeters() == 320.0
        && value.meters() == 3.2;
}

bool testObjectId() {
    constexpr arz::core::ObjectId id = 42;

    return id == 42
        && arz::core::InvalidObjectId == 0;
}

bool testTolerance() {
    const auto& tolerance =
        arz::geometry::defaultTolerance();

    return tolerance.nearlyEqual(
        1000.0,
        1000.0000005
    );
}

bool testPoint2D() {
    const arz::geometry::Point2D a{100.0, 200.0};
    const arz::geometry::Point2D b{100.0, 200.0};

    return a == b;
}

bool testBoundingBox() {
    const arz::geometry::BoundingBox2D box{
        100.0,
        200.0,
        500.0,
        800.0
    };

    return nearlyEqual(box.width(), 400.0)
        && nearlyEqual(box.height(), 600.0)
        && box.contains(250.0, 400.0)
        && !box.contains(50.0, 400.0);
}

bool testDocumentIds() {
    arz::core::Document document;

    const auto first = document.nextObjectId();
    const auto second = document.nextObjectId();

    return first == 1
        && second == 2
        && first != second;
}

bool testDocumentAddLookupRemove() {
    arz::core::Document document;

    const auto id = document.nextObjectId();

    const bool added = document.addObject(
        std::make_unique<TestObject>(id)
    );

    if (!added) {
        return false;
    }

    if (!document.contains(id)) {
        return false;
    }

    if (document.objectCount() != 1) {
        return false;
    }

    const auto* object = document.object(id);

    if (object == nullptr || object->id() != id) {
        return false;
    }

    const bool removed = document.removeObject(id);

    return removed
        && !document.contains(id)
        && document.objectCount() == 0
        && document.object(id) == nullptr;
}

bool testDuplicateObjectRejected() {
    arz::core::Document document;

    const auto id = document.nextObjectId();

    if (!document.addObject(
            std::make_unique<TestObject>(id))) {
        return false;
    }

    const bool duplicateAccepted =
        document.addObject(
            std::make_unique<TestObject>(id)
        );

    return !duplicateAccepted
        && document.objectCount() == 1;
}

bool testInvalidObjectRejected() {
    arz::core::Document document;

    const bool accepted = document.addObject(
        std::make_unique<TestObject>(
            arz::core::InvalidObjectId
        )
    );

    return !accepted
        && document.objectCount() == 0;
}

bool testLineEntity() {
    const arz::geometry::Point2D start{
        0.0,
        0.0
    };

    const arz::geometry::Point2D end{
        3000.0,
        4000.0
    };

    arz::cad::LineEntity line(
        100,
        arz::cad::DefaultLayerId,
        start,
        end
    );

    if (line.entityType() != arz::cad::EntityType::Line) {
        return false;
    }

    if (line.id() != 100) {
        return false;
    }

    if (line.layerId() != arz::cad::DefaultLayerId) {
        return false;
    }

    if (!nearlyEqual(line.length(), 5000.0)) {
        return false;
    }

    const auto box = line.boundingBox();

    return nearlyEqual(box.minX, 0.0)
        && nearlyEqual(box.minY, 0.0)
        && nearlyEqual(box.maxX, 3000.0)
        && nearlyEqual(box.maxY, 4000.0);
}

bool testLineEntityMutation() {
    arz::cad::LineEntity line(
        101,
        arz::cad::DefaultLayerId,
        {0.0, 0.0},
        {1000.0, 0.0}
    );

    line.setEnd({2000.0, 0.0});
    line.setLayerId(9);

    return line.end()
            == arz::geometry::Point2D{2000.0, 0.0}
        && nearlyEqual(line.length(), 2000.0)
        && line.layerId() == 9;
}

bool testLineInsideDocument() {
    arz::core::Document document;

    const auto id = document.nextObjectId();

    auto line =
        std::make_unique<arz::cad::LineEntity>(
            id,
            arz::cad::DefaultLayerId,
            arz::geometry::Point2D{0.0, 0.0},
            arz::geometry::Point2D{1500.0, 0.0}
        );

    if (!document.addObject(std::move(line))) {
        return false;
    }

    const auto* object = document.object(id);

    const auto* storedLine =
        dynamic_cast<const arz::cad::LineEntity*>(
            object
        );

    return storedLine != nullptr
        && storedLine->entityType()
            == arz::cad::EntityType::Line
        && nearlyEqual(
            storedLine->length(),
            1500.0
        );
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

    run("Length", testLength());
    run("ObjectId", testObjectId());
    run("GeometryTolerance", testTolerance());
    run("Point2D", testPoint2D());
    run("BoundingBox2D", testBoundingBox());

    run("DocumentIds", testDocumentIds());
    run(
        "DocumentAddLookupRemove",
        testDocumentAddLookupRemove()
    );
    run(
        "DuplicateObjectRejected",
        testDuplicateObjectRejected()
    );
    run(
        "InvalidObjectRejected",
        testInvalidObjectRejected()
    );

    run("LineEntity", testLineEntity());
    run(
        "LineEntityMutation",
        testLineEntityMutation()
    );
    run(
        "LineInsideDocument",
        testLineInsideDocument()
    );

    if (failures != 0) {
        std::cerr
            << failures
            << " test(s) failed.\n";

        return EXIT_FAILURE;
    }

    std::cout
        << "All core tests passed.\n";

    return EXIT_SUCCESS;
}
