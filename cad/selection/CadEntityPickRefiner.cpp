#include "cad/selection/CadEntityPickRefiner.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "cad/entities/ArcEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/LineEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "geometry/algorithms/Curve2D.h"
#include "geometry/algorithms/Distance2D.h"

namespace arz::cad {
namespace {

template <typename Predicate>
bool forEachCircleBoxIntersection(arz::geometry::Point2D center, double radius,
                                  arz::geometry::BoundingBox2D box,
                                  Predicate predicate) noexcept {
    box = box.normalized();
    const std::array<std::pair<arz::geometry::Point2D, arz::geometry::Point2D>, 4> edges{{
        {{box.minX, box.minY}, {box.maxX, box.minY}},
        {{box.maxX, box.minY}, {box.maxX, box.maxY}},
        {{box.maxX, box.maxY}, {box.minX, box.maxY}},
        {{box.minX, box.maxY}, {box.minX, box.minY}}}};
    for (const auto& [a, b] : edges) {
        const double dx = b.x - a.x, dy = b.y - a.y;
        const double fx = a.x - center.x, fy = a.y - center.y;
        const double qa = dx * dx + dy * dy;
        const double qb = 2.0 * (fx * dx + fy * dy);
        const double qc = fx * fx + fy * fy - radius * radius;
        const double discriminant = qb * qb - 4.0 * qa * qc;
        if (qa == 0.0 || discriminant < 0.0) continue;
        const double root = std::sqrt(std::max(0.0, discriminant));
        for (const double t : {(-qb - root) / (2.0 * qa), (-qb + root) / (2.0 * qa)}) {
            if (t >= 0.0 && t <= 1.0 && predicate(arz::geometry::Point2D{a.x + t * dx, a.y + t * dy}))
                return true;
        }
    }
    return false;
}

bool polylineIntersects(const PolylineEntity& polyline,
                        arz::geometry::BoundingBox2D window) noexcept {
    const auto& vertices = polyline.vertices();
    for (std::size_t i = 1; i < vertices.size(); ++i)
        if (arz::geometry::segmentIntersectsBox(vertices[i - 1], vertices[i], window)) return true;
    return polyline.closed()
        && arz::geometry::segmentIntersectsBox(vertices.back(), vertices.front(), window);
}

}

bool CadEntityPickRefiner::hit(const CadEntity& entity,
    arz::geometry::Point2D point, double tolerance) const noexcept {
    if (tolerance < 0.0 || !std::isfinite(tolerance)) return false;
    if (const auto* line = dynamic_cast<const LineEntity*>(&entity))
        return arz::geometry::distanceToSegment(point, line->start(), line->end()) <= tolerance;
    if (const auto* polyline = dynamic_cast<const PolylineEntity*>(&entity)) {
        const auto& v = polyline->vertices();
        for (std::size_t i = 1; i < v.size(); ++i)
            if (arz::geometry::distanceToSegment(point, v[i - 1], v[i]) <= tolerance) return true;
        return polyline->closed()
            && arz::geometry::distanceToSegment(point, v.back(), v.front()) <= tolerance;
    }
    if (const auto* circle = dynamic_cast<const CircleEntity*>(&entity))
        return arz::geometry::distanceToCircle(point, circle->center(), circle->radius()) <= tolerance;
    if (const auto* arc = dynamic_cast<const ArcEntity*>(&entity))
        return arz::geometry::distanceToArc(point, arc->center(), arc->radius(),
            arc->startAngle(), arc->endAngle(), arc->counterClockwise()) <= tolerance;
    return false;
}

bool CadEntityPickRefiner::intersects(const CadEntity& entity,
    arz::geometry::BoundingBox2D window) const noexcept {
    window = window.normalized();
    if (const auto* line = dynamic_cast<const LineEntity*>(&entity))
        return arz::geometry::segmentIntersectsBox(line->start(), line->end(), window);
    if (const auto* polyline = dynamic_cast<const PolylineEntity*>(&entity))
        return polylineIntersects(*polyline, window);
    if (const auto* circle = dynamic_cast<const CircleEntity*>(&entity)) {
        if (window.contains(circle->boundingBox())) return true;
        return forEachCircleBoxIntersection(circle->center(), circle->radius(), window,
            [](auto) { return true; });
    }
    if (const auto* arc = dynamic_cast<const ArcEntity*>(&entity)) {
        if (window.contains(arc->startPoint().x, arc->startPoint().y)
            || window.contains(arc->endPoint().x, arc->endPoint().y)) return true;
        return forEachCircleBoxIntersection(arc->center(), arc->radius(), window,
            [arc](auto point) {
                return arz::geometry::angleOnArc(std::atan2(point.y - arc->center().y,
                    point.x - arc->center().x), arc->startAngle(), arc->endAngle(),
                    arc->counterClockwise());
            });
    }
    return false;
}

bool CadEntityPickRefiner::contained(const CadEntity& entity,
    arz::geometry::BoundingBox2D window) const noexcept {
    return window.normalized().contains(entity.boundingBox());
}

}
