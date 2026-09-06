#pragma once

#include "cad/entities/EntityGraphicsProperties.h"
#include "cad/layers/LayerId.h"
#include "core/objects/DocumentObject.h"
#include "geometry/primitives/BoundingBox2D.h"

namespace arz::cad {

enum class EntityType {
    Unknown = 0,
    Line,
    Polyline,
    Arc,
    Circle,
    Hatch,
    Text,
    Dimension,
    BlockReference
};

class CadEntity : public arz::core::DocumentObject {
public:
    CadEntity(
        arz::core::ObjectId id,
        LayerId layerId
    );

    ~CadEntity() override = default;

    [[nodiscard]] LayerId layerId() const noexcept;
    void setLayerId(LayerId layerId) noexcept;

    [[nodiscard]] EntityGraphicsProperties&
    graphics() noexcept;

    [[nodiscard]] const EntityGraphicsProperties&
    graphics() const noexcept;

    [[nodiscard]] virtual EntityType
    entityType() const noexcept = 0;

    [[nodiscard]] virtual arz::geometry::BoundingBox2D
    boundingBox() const noexcept = 0;

private:
    LayerId layerId_{DefaultLayerId};
    EntityGraphicsProperties graphics_{};
};

}
