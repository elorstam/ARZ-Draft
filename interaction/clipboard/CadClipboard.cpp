#include "interaction/clipboard/CadClipboard.h"

#include <algorithm>

#include "cad/entities/LineEntity.h"
#include "core/document/Document.h"

namespace arz::interaction {

bool CadClipboard::copy(
    const arz::core::Document& document,
    const std::vector<arz::core::ObjectId>& ids
) {
    std::vector<ClipboardLine> copied;
    for (const auto id : ids) {
        const auto* line = dynamic_cast<const arz::cad::LineEntity*>(document.object(id));
        if (line != nullptr) {
            copied.push_back({line->layerId(), line->start(), line->end(), line->graphics()});
        }
    }
    if (copied.empty()) {
        return false;
    }
    lines_ = std::move(copied);
    basePoint_ = lines_.front().start;
    for (const auto& line : lines_) {
        basePoint_.x = std::min({basePoint_.x, line.start.x, line.end.x});
        basePoint_.y = std::min({basePoint_.y, line.start.y, line.end.y});
    }
    return true;
}

void CadClipboard::clear() noexcept { lines_.clear(); basePoint_ = {}; }
const std::vector<ClipboardLine>& CadClipboard::lines() const noexcept { return lines_; }
bool CadClipboard::empty() const noexcept { return lines_.empty(); }
arz::geometry::Point2D CadClipboard::basePoint() const noexcept { return basePoint_; }

}
