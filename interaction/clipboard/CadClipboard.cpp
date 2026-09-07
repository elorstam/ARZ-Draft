#include "interaction/clipboard/CadClipboard.h"

#include <algorithm>
#include <optional>

#include "cad/entities/LineEntity.h"
#include "cad/entities/CadEntity.h"
#include "cad/entities/PolylineEntity.h"
#include "cad/entities/CircleEntity.h"
#include "cad/entities/ArcEntity.h"
#include "core/document/Document.h"

namespace arz::interaction {

bool CadClipboard::copy(
    const arz::core::Document& document,
    const std::vector<arz::core::ObjectId>& ids
) {
    std::vector<ClipboardLine> copied;
    std::vector<ClipboardPolyline> copiedPolylines;
    std::vector<ClipboardCircle> copiedCircles;
    std::vector<ClipboardArc> copiedArcs;
    std::optional<arz::geometry::BoundingBox2D> bounds;
    for (const auto id : ids) {
        const auto* object = document.object(id);
        const auto* entity = dynamic_cast<const arz::cad::CadEntity*>(object);
        if (entity) {
            const auto current = entity->boundingBox().normalized();
            if (!bounds) bounds = current;
            else {
                bounds->minX = std::min(bounds->minX, current.minX);
                bounds->minY = std::min(bounds->minY, current.minY);
                bounds->maxX = std::max(bounds->maxX, current.maxX);
                bounds->maxY = std::max(bounds->maxY, current.maxY);
            }
        }
        const auto* line = dynamic_cast<const arz::cad::LineEntity*>(object);
        if (line != nullptr) {
            copied.push_back({line->layerId(), line->start(), line->end(), line->graphics()});
        } else if (const auto* polyline = dynamic_cast<const arz::cad::PolylineEntity*>(object)) {
            copiedPolylines.push_back({polyline->layerId(), polyline->vertices(), polyline->closed(), polyline->graphics()});
        } else if (const auto* circle = dynamic_cast<const arz::cad::CircleEntity*>(object)) {
            copiedCircles.push_back({circle->layerId(), circle->center(), circle->radius(), circle->graphics()});
        } else if (const auto* arc = dynamic_cast<const arz::cad::ArcEntity*>(object)) {
            copiedArcs.push_back({arc->layerId(), arc->center(), arc->radius(), arc->startAngle(),
                arc->endAngle(), arc->counterClockwise(), arc->graphics()});
        }
    }
    if (copied.empty() && copiedPolylines.empty() && copiedCircles.empty() && copiedArcs.empty()) {
        return false;
    }
    lines_ = std::move(copied);
    polylines_ = std::move(copiedPolylines);
    circles_ = std::move(copiedCircles);
    arcs_ = std::move(copiedArcs);
    basePoint_ = {bounds->minX, bounds->minY};
    return true;
}

void CadClipboard::clear() noexcept {
    lines_.clear(); polylines_.clear(); circles_.clear(); arcs_.clear(); basePoint_ = {};
}
const std::vector<ClipboardLine>& CadClipboard::lines() const noexcept { return lines_; }
const std::vector<ClipboardPolyline>& CadClipboard::polylines() const noexcept { return polylines_; }
const std::vector<ClipboardCircle>& CadClipboard::circles() const noexcept { return circles_; }
const std::vector<ClipboardArc>& CadClipboard::arcs() const noexcept { return arcs_; }
bool CadClipboard::empty() const noexcept {
    return lines_.empty() && polylines_.empty() && circles_.empty() && arcs_.empty();
}
arz::geometry::Point2D CadClipboard::basePoint() const noexcept { return basePoint_; }

}
