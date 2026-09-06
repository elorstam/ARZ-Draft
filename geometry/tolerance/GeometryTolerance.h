#pragma once

namespace arz::geometry {

struct GeometryTolerance final {
    double linear{1e-6};
    double angular{1e-9};

    [[nodiscard]] bool nearlyEqual(
        double a,
        double b
    ) const;
};

[[nodiscard]] const GeometryTolerance& defaultTolerance();

}
