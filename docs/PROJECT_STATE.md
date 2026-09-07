# PROJECT STATE

## Current Phase
Phase 1.0.1 — CAD Interaction Foundation

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
- Right-click confirm/repeat policy
- Middle-button double-click Zoom Extents
- Durable CAD interaction contract
- Registry-driven command autocomplete with keyboard suggestion navigation
- CAD-style two-click Window/Crossing selection
- Interactive immutable-source paste preview and single-click commit
- Ctrl+V auto-repeat protection and copy/paste geometry-explosion regression coverage

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

## Last Known Status
All Phase 1.0 and Phase 1.0.1 automated tests pass in the Debug MSVC/Qt
configuration. The normal application is launched at the end of the phase for
human acceptance; automated verification does not substitute for that acceptance.

The first Phase 1.0.1 manual review was not accepted. Its command suggestion,
two-click selection, and interactive paste findings have been corrected and await
a new human acceptance pass.

## Spatial Index Synchronization
The spatial index is a derived service outside Document. Phase 0.9
keeps insert, update and remove synchronization explicit after
successful model commands and undo/redo. SelectionService filters
stale candidates against Document, but callers remain responsible
for keeping index bounds current. This avoids coupling document
ownership to the initial index implementation and allows later
automatic synchronization without changing selection query APIs.

## Next Phase
Human acceptance of Phase 1.0.1, then the next approved Phase 1.x milestone.

Goals:
- expand manual drawing and editing commands
- extend entity, selection and snap coverage
- evolve the CAD shell without crossing engine/UI boundaries

## Important
Do not start BIM.
Do not start DWG integration yet.
Do not bypass command/transaction architecture.

## Phase 1.0.1 Acceptance Status

AUTOMATED COMPLETE after the phase commit is pushed. HUMAN ACCEPTED remains
pending until the user verifies direct typing, repeat, prompts, selection modes,
editing shortcuts, drafting toggles, mouse navigation and focus safety in the
normal live application.

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
