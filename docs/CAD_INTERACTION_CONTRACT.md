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
repeating two-point command: a first and second point create one independent
`LineEntity` through its own `AddLineCommand`, then active LINE immediately
returns to `Specify first point`. The completed segment's endpoint is not carried
into the next segment. Enter, Space, or right-click finishes active LINE without
creating a line; only a later idle Enter/Space repeats LINE.

Command persistence does not imply segment continuity. Every completed segment
has its own stable `ObjectId` and undo history entry. LINE does not create a
Polyline or share future PLINE model or connected-chain semantics. Both points of
every independent segment use the shared SnapService while Object Snap is enabled.

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

## Canvas Right-Click Policy

The canvas routes right-click through one context-sensitive policy. While a
point-acquisition command is active, right-click is consumed by the controller as
the command's shared finish/confirm behavior; it never opens a menu. While idle,
right-click opens a native Qt menu near the cursor. The idle menu exposes Repeat
Last Command, Recent Input from the existing command history, a Clipboard submenu,
and Properties. With a selection it additionally exposes Erase, Copy Selection,
Deselect All, and Properties. Each enabled action calls the existing controller,
clipboard, selection, or transaction API; no menu callback mutates Document directly.

## Shortcut and Focus Routing

The application routes Ctrl+Z, Ctrl+Y, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A, Delete,
Escape, drafting F-keys, and direct command typing to the shared controller when
drafting owns input.

Actual text editors take priority. When focus is in `QLineEdit`, `QTextEdit`,
`QPlainTextEdit`, or an editable combo box, Qt native text input and editing
shortcuts are not intercepted. Clicking the canvas restores drafting focus.

Right-click policy is deterministic: while an interactive CAD command is active,
right-click finishes/exits the active command without committing pending geometry.
This clears LINE point state, paste preview, selection-window state, and derived
snap feedback without creating a history entry. While idle, right-click is a safe
no-op reserved for a future context menu; it does not repeat the last command.
Right drag is reserved and does not create selection windows.

## Drafting Settings

One Qt-free drafting settings model backs keyboard shortcuts and status controls:

- F3: Object Snap. This gates real endpoint/midpoint snap queries.
- F7: Grid Display state foundation.
- F8: Ortho state foundation.
- F9: Grid Snap state foundation.
- F10: Polar Tracking state foundation.
- F11: Object Snap Tracking state foundation.
- F12: Dynamic Input. This gates cursor-side command text and prompts.

Ortho and Polar Tracking are mutually exclusive. Foundation-only settings do not
claim geometric behavior that has not been implemented.

## Transient Overlays

Dynamic text, registry suggestions, command prompts, selection/crossing rectangles,
paste previews, snap markers, crosshair, LINE preview, and future tracking graphics are interaction or
view state. They are not document objects, receive no `ObjectId`, cannot be selected
or saved, and do not affect persistent bounds.

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
