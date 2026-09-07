# ARZ Studio CAD Rendering Architecture

## Scope and Boundaries

Phase 1.1 establishes a replaceable screen-rendering pipeline for manual 2D CAD.
Persistent geometry and graphics properties remain Qt-free. QPainter conversion
and drawing live only under `rendering/`. Rendering never mutates `Document`,
entities, layers, graphics properties, selection, or interaction state.

Plot and print lineweight behavior is outside this phase.

## Frame Pipeline

Each canvas paint creates one non-owning `RenderContext` from the current viewport,
screen size, and selected ObjectIds. The context contains:

- the viewport transform;
- visible world bounds derived by `Viewport2D`;
- screen dimensions;
- the current pixels-per-world-unit scale;
- a read-only selection span.

The persistent document pass then follows this sequence:

1. `DocumentRenderPlanner` obtains and explicitly sorts stable ObjectIds.
2. Non-CAD objects and entities on invisible/frozen layers are skipped.
3. Entity bounds are tested against the context's visible world bounds.
4. A registered entity adapter resolves graphics and emits render primitives.
5. `QtDocumentRenderer` converts primitives to QPainter operations.
6. `QtOverlayRenderer` draws all transient interaction graphics afterward.

The canvas does not branch on entity type. Zoom Extents also uses the planner's
visible-document bounds query rather than entity-specific canvas traversal.

## Entity Adapters and Primitives

`IEntityRenderAdapter` is the extension boundary between Qt-free CAD entities and
backend-neutral render primitives. Phase 1.1 registers `LineRenderAdapter`, which
emits `LineRenderPrimitive` values containing ObjectId, endpoints, resolved style,
and a transient selected flag.

Future Polyline, Circle, Arc, Text, Hatch, Dimension, and BlockReference support
adds adapters and primitive/backend handling under `rendering/`; it does not add
entity casts or switches to `CadCanvasWidget`.

The render scene reuses vector capacity across frames. Culling occurs before
graphics resolution. Each visible entity's graphics are resolved once per frame.
No persistent entity pointers are retained by the scene.

## Graphics Resolution

`GraphicsPropertyResolver` remains the authority for ByLayer and explicit color,
linetype, and lineweight. ByBlock has no block context in Phase 1.1 and therefore
retains the existing layer fallback. The renderer converts the resolved RGB value
to `QColor` only in the Qt backend.

Selection uses a transient gold highlight and may widen the screen stroke. It does
not modify resolved or persistent entity graphics.

## Screen Lineweight Policy

`ScreenLineWeightPolicy` maps model lineweight millimeters to display pixels using
a deterministic 96-DPI screen scale (`96 / 25.4` pixels per millimeter). A minimum
1.0-pixel width keeps very thin geometry visible. Default or unresolved special
values use a 0.25 mm display fallback.

Representative mappings are monotonic:

| Model lineweight | Screen width |
| --- | ---: |
| 0.05 mm | 1.00 px |
| 0.09 mm | 1.00 px |
| 0.13 mm | 1.00 px |
| 0.18 mm | 1.00 px |
| 0.25 mm | 1.00 px |
| 0.35 mm | 1.32 px |
| 0.50 mm | 1.89 px |
| 0.70 mm | 2.65 px |
| 1.00 mm | 3.78 px |

This policy is centralized and independent of plotting. A future display setting
can supply a different device scale without changing entity data.

## Culling and Ordering

Render culling is a direct bounding-box intersection against visible world bounds.
It is intentionally separate from the selection/snapping spatial index. Fully
outside entities are skipped; touching or partially visible entities are retained.
No R-tree is introduced in Phase 1.1.

Persistent primitives are rendered in ascending stable ObjectId order. This is an
explicit rendering policy, not an accidental document-storage contract.

## Overlay Pass

The overlay renderer draws LINE previews, paste previews, snap markers,
Window/Crossing rectangles, crosshair, dynamic input, and command suggestions.
These remain outside `Document`, have no ObjectIds, are never culled as persistent
geometry, and render after the document pass.
