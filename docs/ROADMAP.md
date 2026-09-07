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
COMPLETE

Goals:
- Reversible model mutations
- Undo
- Redo
- Transaction history
- No Qt dependency
- Tests for reversible operations

### 0.8 Command API
COMPLETE

Goals:
- Explicit command execution
- LINE creation through command API
- Delete through command API
- Move through command API
- Commands are deterministic and testable
- AI/agents will eventually use the same command API

### 0.9 Selection / Spatial Foundation
COMPLETE

Goals:
- Basic selection queries
- Bounding-box based candidate lookup
- Prepare spatial index architecture
- No brute-force architecture lock-in

### 0.10 Snap Foundation
COMPLETE

Goals:
- Endpoint snap
- Midpoint snap
- Basic nearest-point support
- Snap candidate architecture
- Prepare for future intersection/perpendicular/tangent snaps

---

# Phase 1 — First Interactive CAD

## 1.0 First Interactive CAD Sandbox
COMPLETE

This phase introduces the first live desktop UI for testing the CAD engine.

Technology:
- Qt 6
- Native C++
- Existing ARZ Studio CAD core/document/command architecture

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

A LINE must be drawable interactively using the real ARZ Studio CAD
Document + Command + Transaction + Rendering architecture.

The sandbox is a developer/test interface first.
It does not need the final production UI design.

After Phase 1.0, engine and UI development proceed in parallel.

## 1.0.1 CAD Interaction Foundation

HUMAN ACCEPTED

Established the shared professional CAD interaction contract before adding more
drawing/editing commands:

- centralized command typing, aliases, history, repeat, Enter/Space and Esc
- shared dynamic input and command prompts
- multi-object selection, Window/Crossing and Shift removal
- undoable atomic Delete, Cut and Paste with an internal CAD clipboard
- global focus-safe shortcuts and shared drafting toggles
- right-click routing and Zoom Extents
- registry-driven command autocomplete
- two-click Window/Crossing selection
- interactive immutable-source paste placement

The initial manual review findings were corrected in a follow-up Phase 1.0.1
commit and accepted by the user.

## 1.1 Rendering Foundation

AUTOMATED COMPLETE — HUMAN ACCEPTANCE PENDING

- Qt-free render context and primitive planning
- extensible entity adapters with Line support
- explicit ObjectId draw order and visible-bounds culling
- centralized screen lineweight policy
- separate persistent document and transient overlay passes
- viewport visible-world-bounds support
- repeating LINE sessions producing separate independent LineEntity objects
- repeated endpoint/midpoint-snapped placements from one paste invocation
- shared right-click exit for active interactions with no pending commit

Live-review corrections are automated-verified. Phase 1.2 must not begin until
this phase has been manually accepted.

---

## Phase 1.x — Manual 2D CAD Expansion

NEXT

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

---

## UI Layout Reference

ARZ Studio CAD V1 user interface should follow a familiar professional CAD layout inspired by established desktop CAD workflows, without copying proprietary assets or exact proprietary UI.

Primary layout:

- Top ribbon / tool groups
- Left-side Properties panel
- Large central dark CAD canvas
- Crosshair cursor
- Navigation / view controls near the upper-right of the canvas
- Bottom command line
- Bottom Model / Layout tabs
- Bottom status bar
- Quick toggles for snap, grid, ortho, polar and related drafting modes
- Layer / Color / Linetype / Lineweight controls in the upper tool area

Initial ribbon direction may include:
- Home
- Insert
- Annotate
- Parametric
- View
- Manage
- Output

Important product rule:

Phase 1.0 must not be only a generic debug window.

The first interactive CAD sandbox should already establish the intended CAD shell:

Ribbon + Properties + Canvas + Command Line + Status Bar

The first version may contain only a small number of functional tools, but the overall spatial layout should already resemble a professional desktop CAD application.

ARZ Studio CAD should preserve familiar CAD workflow while using its own visual identity and implementation.

Future ARZ-specific additions may include:
- Story / Floor selector
- Architectural tools
- Area tools
- Parcel / coordinate tools
- Schedule tools
- Project-specific panels

These additions must fit naturally into the CAD shell rather than replacing the core drafting workflow.
