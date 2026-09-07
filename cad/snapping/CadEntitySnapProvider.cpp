#include "cad/snapping/CadEntitySnapProvider.h"

#include <algorithm>
#include <array>

#include "cad/entities/ArcEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/LineEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "geometry/algorithms/Curve2D.h"
#include "geometry/algorithms/Point2DOperations.h"

namespace arz::cad {
namespace {
bool enabled(std::span<const SnapType> types, SnapType type) {
    return std::ranges::find(types, type) != types.end();
}
void addSegmentSnaps(arz::geometry::Point2D first, arz::geometry::Point2D second,
                     bool endpoints, bool midpoint, std::vector<SnapPoint>& points) {
    if (endpoints) {
        points.push_back({SnapType::Endpoint, first});
        if (second != first) points.push_back({SnapType::Endpoint, second});
    }
    if (midpoint) points.push_back({SnapType::Midpoint, arz::geometry::midpoint(first, second)});
}
}

void CadEntitySnapProvider::appendSnapPoints(const CadEntity& entity,
    std::span<const SnapType> types, std::vector<SnapPoint>& points) const {
    const bool endpoints = enabled(types, SnapType::Endpoint);
    const bool midpoints = enabled(types, SnapType::Midpoint);
    if (const auto* line = dynamic_cast<const LineEntity*>(&entity)) {
        addSegmentSnaps(line->start(), line->end(), endpoints, midpoints, points);
        return;
    }
    if (const auto* polyline = dynamic_cast<const PolylineEntity*>(&entity)) {
        const auto& v = polyline->vertices();
        if (endpoints) for (const auto point : v) points.push_back({SnapType::Endpoint, point});
        if (midpoints) {
            for (std::size_t i = 1; i < v.size(); ++i)
                points.push_back({SnapType::Midpoint, arz::geometry::midpoint(v[i - 1], v[i])});
            if (polyline->closed()) points.push_back({SnapType::Midpoint, arz::geometry::midpoint(v.back(), v.front())});
        }
        return;
    }
    if (const auto* circle = dynamic_cast<const CircleEntity*>(&entity)) {
        if (enabled(types, SnapType::Center)) points.push_back({SnapType::Center, circle->center()});
        if (enabled(types, SnapType::Quadrant)) {
            for (const double angle : std::array{0.0, arz::geometry::TwoPi / 4.0,
                    arz::geometry::TwoPi / 2.0, 3.0 * arz::geometry::TwoPi / 4.0})
                points.push_back({SnapType::Quadrant,
                    arz::geometry::pointOnCircle(circle->center(), circle->radius(), angle)});
        }
        return;
    }
    if (const auto* arc = dynamic_cast<const ArcEntity*>(&entity)) {
        if (endpoints) {
            points.push_back({SnapType::Endpoint, arc->startPoint()});
            points.push_back({SnapType::Endpoint, arc->endPoint()});
        }
        if (enabled(types, SnapType::Center)) points.push_back({SnapType::Center, arc->center()});
        if (midpoints) {
            const double sign = arc->counterClockwise() ? 1.0 : -1.0;
            points.push_back({SnapType::Midpoint, arz::geometry::pointOnCircle(
                arc->center(), arc->radius(), arc->startAngle() + sign * arc->sweepAngle() / 2.0)});
        }
        if (enabled(types, SnapType::Quadrant)) {
            for (const double angle : std::array{0.0, arz::geometry::TwoPi / 4.0,
                    arz::geometry::TwoPi / 2.0, 3.0 * arz::geometry::TwoPi / 4.0})
                if (arz::geometry::angleOnArc(angle, arc->startAngle(), arc->endAngle(), arc->counterClockwise()))
                    points.push_back({SnapType::Quadrant,
                        arz::geometry::pointOnCircle(arc->center(), arc->radius(), angle)});
        }
    }
}

}
