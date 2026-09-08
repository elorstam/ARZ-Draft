# PROJECT STATE

## Current Phase
Phase 1.2.5 — CAD INTERACTION COMPATIBILITY LAYER

AUTOMATED COMPLETE — HUMAN ACCEPTANCE PENDING. Phase 1.3 has not started.

## Completed
- C++23 / MSVC / CMake toolchain working
- Core units foundation
- ObjectId
- GeometryTolerance
- Point2D
- BoundingBox2D
- DocumentObject
- ObjectRegistry
- Document
- CadEntity
- LineEntity
- LayerId
- Layer
- LayerTable
- Color
- LineTypeId
- LineWeight
- EntityGraphicsProperties
- GraphicsPropertyResolver
- ICommand
- TransactionHistory
- Command execute / undo / redo lifecycle
- Deterministic undo and redo stacks
- Ownership-preserving object extraction and restoration
- AddLineCommand
- DeleteEntityCommand
- MoveEntityCommand
- BoundingBox2D normalization, intersection and containment
- Point-to-segment distance helper
- ISpatialIndex2D
- LinearSpatialIndex2D
- IEntityPickRefiner
- LineEntityPickRefiner
- SelectionService
- Point2D distance and midpoint helpers
- SnapType
- SnapResult
- IEntitySnapProvider
- LineEntitySnapProvider
- SnapService
- Deterministic snap candidate ranking
- Qt 6 Widgets application integration
- Professional CAD desktop shell
- Qt-free viewport transformation
- Qt-free command-line parser
- Qt-free LINE input state machine
- CadApplicationController
- Explicit spatial index rebuild synchronization
- QPainter document renderer
- Interactive pan, zoom, coordinates and crosshair
- LINE drawing with endpoint and midpoint snap feedback
- Click selection and read-only Line properties
- Real transaction undo/redo integration
- Central Qt-free CAD command registry, input buffer, history and repeat state
- Direct command typing from the drafting canvas with shared dynamic input
- Context-sensitive Enter/Space and prioritized Esc routing
- Deterministic multi-object selection set outside Document ownership
- CAD Window and Crossing selection with Shift removal
- Atomic multi-object Delete, Cut and Paste transactions
- Internal Line geometry clipboard with new paste ObjectIds
- Shared F3/F7/F8/F9/F10/F11/F12 drafting settings and status controls
- OSNAP and dynamic-input functional toggles
- Focus-safe application shortcut routing
- Shared right-click active-interaction finish policy with idle no-op behavior
- Middle-button double-click Zoom Extents
- Durable CAD interaction contract
- Registry-driven command autocomplete with keyboard suggestion navigation
- CAD-style two-click Window/Crossing selection
- Interactive immutable-source paste preview and single-click commit
- Ctrl+V auto-repeat protection and copy/paste geometry-explosion regression coverage
- Qt-free render context and backend-neutral render primitives
- Extensible entity render adapter boundary with LineRenderAdapter
- Explicit stable ObjectId render ordering
- Visible-world-bounds culling before graphics resolution
- Central screen lineweight display policy
- Separate QPainter document and transient overlay passes
- Viewport visible-world-bounds calculation
- Canvas free of entity-specific rendering and Zoom Extents branches
- Reused render-scene storage for paint-loop efficiency
- Chained LINE sessions creating separate, connected, individually undoable LineEntity objects
- Repeated endpoint/midpoint-snapped paste placement from one immutable clipboard snapshot
- Right-click exit for LINE and repeated paste without geometry or history mutation
- PolylineEntity, CircleEntity and ArcEntity geometry with stable ObjectIds
- Explicit undoable add commands for PLINE, CIRCLE and three-point ARC
- Qt-free input controllers and live previews for the three new drawing commands
- Entity render adapters and backend-neutral primitives for polylines, circles and arcs
- Shape-refined point, Window and Crossing selection
- Polyline vertex/segment, circle center/quadrant and arc endpoint/midpoint/center/quadrant snaps
- Read-only entity-specific properties for polylines, circles and arcs
- Atomic mixed-entity clipboard copy, preview, paste, undo and redo
- Registry autocomplete and PL, C and A aliases
- Functional ribbon actions for Polyline, Circle and Arc
- Shared transient snap candidates for active PLINE vertices and segment midpoints
- True three-state ARC with live mathematical/reference preview and safe collinear handling
- Native idle/selection canvas context menu with repeat, history, clipboard, erase and properties routing
- Distinct continuous COPY/CO workflow with a fixed base point and immutable source, separate from Ctrl+C clipboard copy
- Compact 36-pixel-arm CAD crosshair and centered 8-pixel pickbox rendered as transient cursor overlay
- Immediate post-P2 ARC reference overlay with safe collinear construction guidance
- Immediate post-P1 ARC second-point rubber-band reference with shared OSNAP gating
- Mini top-ribbon/header refinement with original ARZ Studio CAD branding and glyphs
- Recalibrated 168 px top shell: 27 px header, 25 px tabs, 91 px ribbon, 25 px document tabs
- Eleven-tab CAD ribbon with dense Home groups and structured non-Home placeholders
- Tightened 28/16 px icon hierarchy, 9 px ribbon typography, panel padding and control spacing
- Qt-free shared `PointAcquisition` result with raw, snapped, constrained, and final points
- Shared `InteractionStage` and right-click policy routing
- Shared structured dynamic-input state for prompt, coordinate, distance, angle, snap, and options
- Controller-owned transient LINE rubber band and snap marker source
- F3/F8 immediate provisional-point recomputation
- Ortho preview/commit constraints across LINE, PLINE, CIRCLE, ARC, COPY, and paste point stages
- Chained LINE workflow with independent segment IDs and undo entries
- LINE `Undo` option restoring the previous chain point
- Preview/commit identity and no-document-mutation regression coverage
- AutoCAD 2021 LINE black-box behavioral reference documentation

## Tests
Passing:
- core_tests
- layer_tests
- graphics_tests
- transaction_tests
- command_tests
- selection_tests
- snap_tests
- interactive_foundation_tests
- cad_interaction_tests
- ui_shell_tests
- ui_input_routing_tests
- rendering_foundation_tests
- phase_1_2_tests
- cad_interaction_compatibility_tests

## Last Known Status
Phases 1.0 and 1.0.1 are HUMAN ACCEPTED. Phase 1.2 remains CLOSED / ACCEPTED WITH
KNOWN ISSUES. Phase 1.2.5 adds the shared interaction compatibility layer without
starting Phase 1.3. The complete 14-test Debug MSVC/Qt suite passes. Automated
verification does not substitute for live human acceptance.

## Spatial Index Synchronization
The spatial index is a derived service outside Document. Phase 0.9
keeps insert, update and remove synchronization explicit after
successful model commands and undo/redo. SelectionService filters
stale candidates against Document, but callers remain responsible
for keeping index bounds current. This avoids coupling document
ownership to the initial index implementation and allows later
automatic synchronization without changing selection query APIs.

## Next Phase
Human retest of Phase 1.2.5, followed only by the next explicitly approved Phase
1.x milestone. Phase 1.3 has not started.

## Phase 1.2.5 Known Limitations

- Active-command right-click preserves direct convenient finish rather than the
  complete AutoCAD-style context menu.
- Dynamic input exposes structured prompt/coordinate/distance/angle/options data
  but fields are not directly editable.
- Object snap targets take precedence over Ortho; advanced tracking and temporary
  snap overrides are not implemented.
- Shared selection stages are defined for reuse, while modify-command selection
  consumption remains Phase 1.3 work.

Goals:
- expand manual drawing and editing commands
- extend entity, selection and snap coverage
- evolve the CAD shell without crossing engine/UI boundaries

## Important
Do not start BIM.
Do not start DWG integration yet.
Do not bypass command/transaction architecture.

## Phase 1.2 Acceptance Status

CLOSED / ACCEPTED WITH KNOWN ISSUES. The current mini UI pass refines only the
top header, ribbon, and drawing-tab shell; it does not reopen Phase 1.2 scope or
start Phase 1.3.

## Locked UI Direction

The first interactive ARZ Studio CAD UI will use a professional desktop CAD layout.

Reference structure:
- Ribbon / tool groups at top
- Properties panel at left
- Central dark drafting canvas
- Crosshair
- Command line at bottom
- Model / Layout tabs
- CAD status bar and drafting toggles

Phase 1.0 is expected to establish this shell even before every tool is implemented.

Do not replace this with a generic single-canvas debug window unless temporarily required for isolated testing.

The UI should feel familiar to AutoCAD users while remaining visually and technically original to ARZ Studio CAD.
