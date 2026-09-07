#include "interaction/clipboard/CadClipboard.h"

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
    pasteGeneration_ = 0;
    return true;
}

void CadClipboard::clear() noexcept { lines_.clear(); pasteGeneration_ = 0; }
const std::vector<ClipboardLine>& CadClipboard::lines() const noexcept { return lines_; }
bool CadClipboard::empty() const noexcept { return lines_.empty(); }
std::size_t CadClipboard::pasteGeneration() const noexcept { return pasteGeneration_; }
void CadClipboard::advancePasteGeneration() noexcept { ++pasteGeneration_; }

}
