# ARZ Draft Roadmap

## Phase 0 — Core Foundations

### 0.1 Bootstrap
COMPLETE

### 0.2 Object Identity / Tolerance / Tests
COMPLETE

### 0.3 Document Core
COMPLETE

### 0.4 CAD Entity Foundation
COMPLETE

### 0.5 Layer Foundation
COMPLETE

### 0.6 Entity Graphics Properties
COMPLETE

### 0.7 Transaction / Undo-Redo
NEXT

Goals:
- Reversible model mutations
- Undo
- Redo
- Transaction history
- No Qt dependency
- Tests for reversible operations

### 0.8 Command API
PLANNED

Goals:
- Explicit command execution
- LINE creation through command API
- Delete through command API
- Move through command API
- Commands are deterministic and testable
- AI/agents will eventually use the same command API

### 0.9 Selection / Spatial Foundation
PLANNED

Goals:
- Basic selection queries
- Bounding-box based candidate lookup
- Prepare spatial index architecture
- No brute-force architecture lock-in

### 0.10 Snap Foundation
PLANNED

Goals:
- Endpoint snap
- Midpoint snap
- Basic nearest-point support
- Snap candidate architecture
- Prepare for future intersection/perpendicular/tangent snaps

---

# Phase 1 — First Interactive CAD

## 1.0 First Interactive CAD Sandbox
PLANNED

This phase introduces the first live desktop UI for testing the CAD engine.

Technology:
- Qt 6
- Native C++
- Existing ARZ Draft core/document/command architecture

Initial UI goals:
- Main desktop window
- Dark CAD canvas
- Crosshair cursor
- Mouse coordinate display
- Pan
- Zoom
- Basic grid
- Command line
- Current layer selector
- Basic entity rendering

First end-to-end workflow:

1. User types LINE
2. User selects first point
3. User selects second point
4. Command API creates a LineEntity
5. LineEntity is stored in Document
6. Renderer displays the entity
7. Ctrl+Z removes it through Undo
8. Ctrl+Y restores it through Redo

Acceptance goal:

A LINE must be drawable interactively using the real ARZ Draft
Document + Command + Transaction + Rendering architecture.

The sandbox is a developer/test interface first.
It does not need the final production UI design.

After Phase 1.0, engine and UI development proceed in parallel.

---

## Phase 1.x — Manual 2D CAD Expansion

Planned:
- POLYLINE
- CIRCLE
- ARC
- MOVE
- COPY
- ROTATE
- MIRROR
- OFFSET
- TRIM
- EXTEND
- FILLET
- STRETCH
- Advanced object snaps
- Layer manager
- Color / Linetype / Lineweight UI
- Blocks
- Hatch
- Text / MText
- Dimensions
- Layout / Paper Space
- PDF output

Architectural workflow tools will be added progressively.

---

## V1 Product Goal

Professional manual 2D architectural CAD focused on real
architectural office workflows.

Includes:
- Story / floor organization
- Architectural area calculations
- Parcel tools
- Coordinate import
- Setback / buildable boundary calculations
- Architectural schedules
- Independent unit schedules
- DXF / DWG interoperability
- Layouts and PDF output

---

## V2 Product Goal

BIM and model-derived architectural views.

Planned:
- Walls
- Doors
- Windows
- Slabs
- Roofs
- Levels
- Automatic sections
- Automatic elevations
- Quantity takeoff
- OpenCascade-based advanced geometry
