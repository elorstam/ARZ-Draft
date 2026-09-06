# PROJECT STATE

## Current Phase
Phase 0.9 — Selection / Spatial Foundation

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

## Tests
Passing:
- core_tests
- layer_tests
- graphics_tests
- transaction_tests
- command_tests
- selection_tests

## Last Known Status
All Phase 0.9 tests passed.

## Spatial Index Synchronization
The spatial index is a derived service outside Document. Phase 0.9
keeps insert, update and remove synchronization explicit after
successful model commands and undo/redo. SelectionService filters
stale candidates against Document, but callers remain responsible
for keeping index bounds current. This avoids coupling document
ownership to the initial index implementation and allows later
automatic synchronization without changing selection query APIs.

## Next Phase
Phase 0.10 — Snap Foundation

Goals:
- endpoint snap
- midpoint snap
- basic nearest-point support
- extensible snap candidate architecture

## Important
Do not start BIM.
Do not start Qt UI yet.
Do not start DWG integration yet.
Do not bypass command/transaction architecture.

## Locked UI Direction

The first interactive ARZ Draft UI will use a professional desktop CAD layout.

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

The UI should feel familiar to AutoCAD users while remaining visually and technically original to ARZ Draft.
