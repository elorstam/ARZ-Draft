#pragma once

namespace arz::core {
class Document;
}

namespace arz::cad {

class ISpatialIndex2D;

class SpatialIndexSynchronizer final {
public:
    [[nodiscard]] static bool rebuild(
        const arz::core::Document& document,
        ISpatialIndex2D& spatialIndex
    );
};

}
