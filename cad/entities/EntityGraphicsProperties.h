#pragma once

#include "cad/entities/GraphicsPropertySource.h"
#include "cad/layers/Color.h"
#include "cad/layers/LineTypeId.h"
#include "cad/layers/LineWeight.h"

namespace arz::cad {

struct EntityGraphicsProperties final {
    GraphicsPropertySource colorSource{
        GraphicsPropertySource::ByLayer
    };

    Color color{
        Color::white()
    };

    GraphicsPropertySource lineTypeSource{
        GraphicsPropertySource::ByLayer
    };

    LineTypeId lineTypeId{
        ContinuousLineTypeId
    };

    GraphicsPropertySource lineWeightSource{
        GraphicsPropertySource::ByLayer
    };

    LineWeight lineWeight{
        LineWeight::Default
    };
};

struct ResolvedGraphicsProperties final {
    Color color;
    LineTypeId lineTypeId;
    LineWeight lineWeight;
};

}
