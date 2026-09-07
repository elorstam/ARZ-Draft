#include "geometry/algorithms/Curve2D.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "geometry/algorithms/Point2DOperations.h"

namespace arz::geometry {

double normalizeAngle(double angle) noexcept {
    if (!std::isfinite(angle)) return 0.0;
    angle = std::fmod(angle, TwoPi);
    return angle < 0.0 ? angle + TwoPi : angle;
}

double directedAngleSweep(double start, double end,
                          bool counterClockwise) noexcept {
    const double ccw = normalizeAngle(end - start);
    if (counterClockwise) return ccw;
    return ccw == 0.0 ? 0.0 : TwoPi - ccw;
}

bool angleOnArc(double angle, double start, double end,
                bool counterClockwise, double tolerance) noexcept {
    angle = normalizeAngle(angle);
    start = normalizeAngle(start);
    end = normalizeAngle(end);
    const double sweep = directedAngleSweep(start, end, counterClockwise);
    const double candidate = counterClockwise
        ? normalizeAngle(angle - start) : normalizeAngle(start - angle);
    return candidate <= sweep + std::max(0.0, tolerance);
}

Point2D pointOnCircle(Point2D center, double radius, double angle) noexcept {
    return {center.x + radius * std::cos(angle),
            center.y + radius * std::sin(angle)};
}

std::optional<Circumcircle2D> circumcircle(
    Point2D a, Point2D b, Point2D c, double tolerance) noexcept {
    const double determinant = 2.0 * (a.x * (b.y - c.y)
        + b.x * (c.y - a.y) + c.x * (a.y - b.y));
    const double scale = std::max({1.0, distance(a, b), distance(b, c),
                                   distance(c, a)});
    if (!std::isfinite(determinant)
        || std::abs(determinant) <= std::max(0.0, tolerance) * scale) {
        return std::nullopt;
    }
    const double aa = a.x * a.x + a.y * a.y;
    const double bb = b.x * b.x + b.y * b.y;
    const double cc = c.x * c.x + c.y * c.y;
    const Point2D center{
        (aa * (b.y - c.y) + bb * (c.y - a.y) + cc * (a.y - b.y)) / determinant,
        (aa * (c.x - b.x) + bb * (a.x - c.x) + cc * (b.x - a.x)) / determinant};
    const double radius = distance(center, a);
    if (!std::isfinite(center.x) || !std::isfinite(center.y)
        || !std::isfinite(radius) || radius <= std::max(0.0, tolerance)) {
        return std::nullopt;
    }
    return Circumcircle2D{center, radius};
}

double polylineLength(const std::vector<Point2D>& vertices,
                      bool closed) noexcept {
    double result = 0.0;
    for (std::size_t i = 1; i < vertices.size(); ++i)
        result += distance(vertices[i - 1], vertices[i]);
    if (closed && vertices.size() > 2) result += distance(vertices.back(), vertices.front());
    return result;
}

BoundingBox2D polylineBounds(const std::vector<Point2D>& vertices) noexcept {
    if (vertices.empty()) return {};
    BoundingBox2D result{vertices.front().x, vertices.front().y,
                         vertices.front().x, vertices.front().y};
    for (const auto point : vertices) {
        result.minX = std::min(result.minX, point.x);
        result.minY = std::min(result.minY, point.y);
        result.maxX = std::max(result.maxX, point.x);
        result.maxY = std::max(result.maxY, point.y);
    }
    return result;
}

BoundingBox2D circleBounds(Point2D center, double radius) noexcept {
    radius = std::max(0.0, radius);
    return {center.x - radius, center.y - radius,
            center.x + radius, center.y + radius};
}

BoundingBox2D arcBounds(Point2D center, double radius, double start,
                        double end, bool counterClockwise) noexcept {
    const auto first = pointOnCircle(center, radius, start);
    const auto last = pointOnCircle(center, radius, end);
    BoundingBox2D result{std::min(first.x, last.x), std::min(first.y, last.y),
                         std::max(first.x, last.x), std::max(first.y, last.y)};
    constexpr std::array quadrants{0.0, TwoPi / 4.0, TwoPi / 2.0,
                                   3.0 * TwoPi / 4.0};
    for (const double angle : quadrants) {
        if (!angleOnArc(angle, start, end, counterClockwise)) continue;
        const auto point = pointOnCircle(center, radius, angle);
        result.minX = std::min(result.minX, point.x);
        result.minY = std::min(result.minY, point.y);
        result.maxX = std::max(result.maxX, point.x);
        result.maxY = std::max(result.maxY, point.y);
    }
    return result;
}

double distanceToCircle(Point2D point, Point2D center, double radius) noexcept {
    return std::abs(distance(point, center) - std::max(0.0, radius));
}

double distanceToArc(Point2D point, Point2D center, double radius,
                     double start, double end, bool counterClockwise) noexcept {
    const double angle = std::atan2(point.y - center.y, point.x - center.x);
    if (angleOnArc(angle, start, end, counterClockwise))
        return distanceToCircle(point, center, radius);
    return std::min(distance(point, pointOnCircle(center, radius, start)),
                    distance(point, pointOnCircle(center, radius, end)));
}

bool segmentIntersectsBox(Point2D start, Point2D end, BoundingBox2D box) noexcept {
    box = box.normalized();
    if (box.contains(start.x, start.y) || box.contains(end.x, end.y)) return true;
    double t0 = 0.0;
    double t1 = 1.0;
    const double dx = end.x - start.x;
    const double dy = end.y - start.y;
    const std::array p{-dx, dx, -dy, dy};
    const std::array q{start.x - box.minX, box.maxX - start.x,
                       start.y - box.minY, box.maxY - start.y};
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (p[i] == 0.0) {
            if (q[i] < 0.0) return false;
            continue;
        }
        const double r = q[i] / p[i];
        if (p[i] < 0.0) t0 = std::max(t0, r);
        else t1 = std::min(t1, r);
        if (t0 > t1) return false;
    }
    return true;
}

}
