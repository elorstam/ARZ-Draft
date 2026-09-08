# ARZ Studio CAD Interaction Contract

## Purpose

Every drafting command participates in one shared interaction contract. Geometry
success alone does not complete a command. Command typing, keyboard confirmation,
cancellation, selection workflows, shortcuts, drafting state, undo/redo, mouse
input, prompts, and transient graphics must remain coherent.

The Qt-free `CadApplicationController` owns the interaction state used by the UI.
Widgets translate Qt events into controller operations; they do not parse commands,
own persistent selections, or mutate the `Document`.

## Command Input Lifecycle

The central command registry stores canonical names, aliases, and repeatability.
Only implemented commands are registered. LINE currently has alias L and is
repeatable. UNDO, REDO, and CANCEL/ESC are non-repeatable.

Printable canvas input is normalized to uppercase and appended to the shared
command buffer. Backspace removes one character. The bottom command edit is an
editor/view of that same state. Dynamic input renders the same buffer near the
cursor. A command invocation is recorded by canonical name only after successful
dispatch.

The registry also supplies case-insensitive prefix suggestions across canonical
names and aliases. The transient suggestion list filters after every character or
Backspace, selects the best deterministic result by default, and supports cyclic
Up/Down navigation. Enter or Space invokes the selected canonical command. Adding
a registered command automatically makes it eligible without widget changes.

Enter and Space use this state order:

1. A nonempty buffer is resolved and invoked.
2. Otherwise, an active command receives confirm/default/finish.
3. Otherwise, the last successfully invoked repeatable command starts again.
4. With no applicable state, the input is a safe no-op.

An active command owns the meaning of confirmation at each step. LINE is a
continuous chained command: P1-P2 creates one independent `LineEntity`, P2 becomes
the next fixed point, and P2-P3 creates another. Each segment still has its own
stable `ObjectId` and undo-history entry. Enter, Space, or the active-command
right-click policy finishes LINE without creating the unfinished segment; a later
idle Enter/Space repeats LINE through the command registry.

LINE exposes `Undo` after its first committed segment. The option undoes one LINE
segment transaction and restores the preceding chain point. Normal application
undo remains one transaction at a time and does not combine a chain into a
Polyline or one aggregate history item.

PLINE owns a connected vertex sequence and finishes open with Enter/right-click,
or closed with the `Close` option (`C`). CIRCLE uses center plus radius point. ARC
uses three points in start, through, end order. All point stages share SnapService,
all pending geometry stays in OverlayState, and only a complete entity reaches the
Document through its explicit transaction command.

While PLINE is active, the shared SnapService ranking pipeline also receives
ObjectId-free transient candidates for every acquired vertex and segment midpoint.
They are combined with document candidates, ranked by the same deterministic policy,
and discarded when the command ends. F3 disables both sources. The active geometry
never enters Document or the spatial index.

ARC remains active after the first two points. Cursor movement after P2 creates only
a transient three-point arc preview, reference construction lines, and dynamic
radius/sweep information. Only a confirmed third point creates an ArcEntity;
Enter/right-click before P3 cancels the provisional state without mutation, and
collinear provisional input is safely hidden.

## Cancellation

Escape is idempotent and follows this priority:

1. Cancel a pending two-click selection rectangle.
2. Cancel transient paste placement without model mutation.
3. Cancel the active command and its current step data.
4. Clear the typed command buffer and suggestions.
5. Clear the selection set.
6. Remain idle.

Undo and redo safely cancel active drawing input and remove stale selection IDs after
spatial-index synchronization.

## Selection

Selection state is a deterministic ordered set of `ObjectId` values outside the
`Document`. It never owns entities or stores persistent raw pointers. Stale IDs are
removed after model transactions.

- A hit click adds an entity to the selection set.
- An empty click clears selection unless Shift is held.
- Shift+click toggles the hit entity.
- Ctrl+A selects visible, unfrozen, unlocked supported CAD entities.
- Properties show entity details only for exactly one supported entity; multiple
  selection is reported as a count.

An empty first click stores the first selection corner. Mouse movement updates the
opposite transient corner without a held button, and a second click commits.
Left-to-right uses `SelectionService::containedWindow`: only completely contained
entities are included. Right-to-left uses `SelectionService::crossingWindow`:
contained and intersecting entities are included. Shift on the two-click operation
removes matching entities. A direct entity hit remains a point pick and never
starts the rectangle. The controller chooses the query; the widget only supplies
click and pointer coordinates.

The selection model is deliberately independent of command state. Future editing
commands can consume a preselection, or enter a Select objects state and use the
same click/window operations before Enter/Space confirms the selection.

## Transactions and Clipboard

Delete never mutates the model from a key handler. One `DeleteEntitiesCommand`
extracts all selected supported entities as one history entry. Undo restores their
original objects and IDs; redo removes them again.

The internal CAD clipboard stores immutable deterministic value snapshots for Line,
Polyline, Circle, and Arc entities. It stores layer, geometry, graphics data, and a base
point at the copied set's minimum bounds corner—never entity pointers. Copy does
not mutate the document. Cut is copy followed by one undoable multi-delete
transaction.

Ctrl+V enters transient repeated paste placement without creating document
objects. Every pointer update computes `preview = immutable clipboard source +
absolute placement delta`; preview data is never used as the next source. Each
click (or confirm at the current insertion point) constructs one final copied set
and executes one mixed-entity add command, then paste placement remains active for the
next location. Every placement receives new IDs and is an independent undo entry.
Escape discards the current preview and exits paste without another mutation. The
stored base point is the copied geometry's minimum bounds corner and is ready for
future COPYBASE support. Paste placement uses the shared endpoint/midpoint
`SnapService` when Object Snap is enabled. Every preview and commit uses the
resolved insertion point minus the clipboard base point. With Object Snap
disabled, every placement uses the raw cursor world point.

One paste invocation supports repeated placements until explicitly finished.
Right-click or Escape exits repeated paste without committing the current preview.
Enter and Space retain their paste placement meaning: they commit once at the
current resolved insertion point and leave paste active.

Clipboard Copy (`Ctrl+C` or Clipboard > Copy) only stores the immutable snapshot;
it never starts placement. CAD COPY (`COPY`, `CO`, or Selection > Copy Selection)
is a separate interactive command: it asks for one base point, previews displacement
to each destination, and commits each new-ID copied set as its own undoable action.
COPY remains active for additional destinations, always using the immutable original
snapshot and fixed base point; transforms never accumulate. Enter, Space, right-click,
or Escape finishes the COPY session. Originals remain unchanged.

## Canvas Right-Click Policy

The canvas routes right-click through `InteractionStage` and one Qt-free
`CommandInputPolicy`. Idle and ordinary selected-object stages request a native
context menu. Awaiting-point and repeating-destination stages finish the active
interaction. Awaiting-selection-confirmation confirms the shared selection stage,
and a context-menu-open stage is a no-op. Widgets construct menus but do not decide
command semantics.

The idle menu exposes Repeat
Last Command, Recent Input from the existing command history, a Clipboard submenu,
and Properties. With a selection it additionally exposes Erase, Copy Selection,
Deselect All, and Properties. Each enabled action calls the existing controller,
clipboard, selection, or transaction API; no menu callback mutates Document directly.

ARZ currently preserves the convenient direct right-click finish for active LINE,
PLINE, COPY, paste, CIRCLE, and ARC interactions. It does not yet show the full
AutoCAD-style active-command context menu; adding that menu later must continue to
route its Enter/Cancel actions through this policy.

## Shortcut and Focus Routing

The application routes Ctrl+Z, Ctrl+Y, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A, Delete,
Escape, drafting F-keys, and direct command typing to the shared controller when
drafting owns input.

Actual text editors take priority. When focus is in `QLineEdit`, `QTextEdit`,
`QPlainTextEdit`, or an editable combo box, Qt native text input and editing
shortcuts are not intercepted. Clicking the canvas restores drafting focus.

Right-click policy is deterministic: while an interactive CAD command is active,
right-click is consumed by that command's shared finish/confirm contract. While
idle, it opens the native context menu; selection actions route through controller
commands, including interactive CAD COPY rather than clipboard Copy.
Right drag is reserved and does not create selection windows.

## Drafting Settings

One Qt-free drafting settings model backs keyboard shortcuts and status controls:

- F3: Object Snap. This gates real endpoint/midpoint snap queries.
- F7: Grid Display state foundation.
- F8: Ortho constraint for current point-based command previews and commits.
- F9: Grid Snap state foundation.
- F10: Polar Tracking state foundation.
- F11: Object Snap Tracking state foundation.
- F12: Dynamic Input. This gates cursor-side command text and prompts.

Ortho and Polar Tracking are mutually exclusive. F3 and F8 immediately recompute
the current provisional point from the last raw pointer sample.

## Shared Point Acquisition

`PointAcquisition` is the Qt-free reusable resolution stage for LINE, PLINE,
CIRCLE, ARC, CAD COPY, and clipboard paste. The controller supplies document and
transient snap candidates through `SnapService`; the point stage itself performs
no model mutation.

The deterministic data path is:

1. Retain the raw pointer point.
2. Retain an optional ranked object-snap result.
3. Apply the stage constraint from its fixed origin when no explicit snap exists.
4. Publish one `ResolvedCadPoint` for preview, marker, dynamic input, and commit.

A positive object snap is an explicit geometric target and takes precedence over
Ortho. If there is no snap, Ortho selects the dominant horizontal or vertical axis
from the stage origin; an exact tie resolves horizontally. A displayed snap marker
therefore always identifies the actual commit point.

Every click and every pointer preview calls the same resolver. `OverlayState`
stores the resulting raw, snapped, constrained, final, and snap-marker data. The
canvas no longer owns a separate LINE preview or snap pipeline.

## Shared Interaction Stages and Selection Acquisition

The reusable stages currently distinguish Idle, Selection Active, Awaiting
Selection Confirmation, Awaiting Point, Awaiting Repeating Destination, and
Context Menu Open. Future modify commands consume these stage semantics rather
than assigning global meanings to Enter, Space, Escape, or right-click.

The reusable selection acquisition contract is:

`SelectObjects -> SelectionSet -> Enter/Space/right-click confirmation -> next command stage`

Existing point, Window, Crossing, Shift add/remove, and preselection behavior is
unchanged. Selection remains outside `Document`; future command-first selection
may consume the same set and confirmation transition.

## Transient Overlays

Dynamic text, registry suggestions, command prompts, selection/crossing rectangles,
paste/COPY previews, snap markers, and future tracking graphics are interaction or
view state. They are not document objects, receive no `ObjectId`, cannot be selected
or saved, and do not affect persistent bounds.

`OverlayState` is the shared transient contract for LINE rubber-band geometry,
PLINE/CIRCLE/ARC previews, COPY/paste transformed geometry, snap markers,
construction references, selection windows, and dynamic input. These values have
no `ObjectId`, never enter the spatial index or undo history, and pointer movement
cannot call a document command. Regression coverage executes hundreds of pointer
moves while asserting unchanged document object and history counts.

Dynamic input has a structured Qt-free state containing the current prompt,
resolved coordinate, optional distance and angle, optional snap type, and command
options. LINE populates distance and angle after P1. Rendering remains deliberately
compact and does not attempt to clone AutoCAD's editable multi-field heads-up UI.

The drafting canvas renders a light professional crosshair and centered square
pickbox as a transient overlay while hiding the native arrow cursor. Its configurable
defaults use 36-pixel arms and an 8-pixel pickbox rather than viewport-spanning lines. Snap markers
remain visually distinct from the pickbox and dynamic input is cursor-relative.

After ARC P1, overlay state immediately enters its second-point reference stage and
renders a snapped/raw P1-to-provisional-P2 rubber band on every pointer update. After
P2, it switches directly to the third-point reference stage with fixed P1/P2 and
provisional P3. Safe construction lines render even when those points are collinear;
the optional arc, radius, and sweep values appear only when the circumcircle is valid.

Window selection is blue and solid; crossing selection is green and dashed.
Middle-button drag pans, wheel input zooms about the cursor, and middle-button
double-click fits visible supported entity bounds without document mutation.

## Command Responsibilities

Each future command must declare its canonical name, aliases, and repeatability;
provide its state-specific prompt; handle confirm/default/finish and cancellation;
consume preselection or request selection through the shared selection set; execute
model changes through explicit atomic command APIs; and expose only transient
preview state outside the `Document`.

Widgets must not add command-specific parsers, aliases, persistent state, or model
mutation shortcuts.

## AutoCAD 2021 Behavioral Reference and Known Deviations

Phase 1.2.5 used black-box LINE observations documented in
`docs/reference/autocad/LINE_2021.md`. No Autodesk code or assets are included.

Known deviations are intentional phase limits: ARZ directly finishes active
commands on right-click instead of showing the complete active LINE context menu;
dynamic input is display-only rather than an editable coordinate/distance/angle
entry system; Recent Input, Osnap Overrides, Pan, Zoom, and QuickCalc-style active
menu items are not implemented; advanced tracking and snap overrides remain
future work.
