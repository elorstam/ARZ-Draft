#pragma once

#include <optional>
#include <vector>

#include "geometry/primitives/BoundingBox2D.h"
#include "geometry/primitives/Point2D.h"

namespace arz::geometry {

inline constexpr double TwoPi = 6.283185307179586476925286766559;

struct Circumcircle2D final {
    Point2D center{};
    double radius{};
};

[[nodiscard]] double normalizeAngle(double angle) noexcept;
[[nodiscard]] double directedAngleSweep(double start, double end,
                                         bool counterClockwise) noexcept;
[[nodiscard]] bool angleOnArc(double angle, double start, double end,
                              bool counterClockwise,
                              double tolerance = 1e-9) noexcept;
[[nodiscard]] Point2D pointOnCircle(Point2D center, double radius,
                                    double angle) noexcept;
[[nodiscard]] std::optional<Circumcircle2D> circumcircle(
    Point2D first, Point2D second, Point2D third,
    double tolerance = 1e-6) noexcept;
[[nodiscard]] double polylineLength(const std::vector<Point2D>& vertices,
                                    bool closed) noexcept;
[[nodiscard]] BoundingBox2D polylineBounds(
    const std::vector<Point2D>& vertices) noexcept;
[[nodiscard]] BoundingBox2D circleBounds(Point2D center,
                                         double radius) noexcept;
[[nodiscard]] BoundingBox2D arcBounds(Point2D center, double radius,
                                      double start, double end,
                                      bool counterClockwise) noexcept;
[[nodiscard]] double distanceToCircle(Point2D point, Point2D center,
                                      double radius) noexcept;
[[nodiscard]] double distanceToArc(Point2D point, Point2D center,
                                   double radius, double start, double end,
                                   bool counterClockwise) noexcept;
[[nodiscard]] bool segmentIntersectsBox(Point2D start, Point2D end,
                                        BoundingBox2D box) noexcept;

}
