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

Enter and Space use this state order:

1. A nonempty buffer is resolved and invoked.
2. Otherwise, an active command receives confirm/default/finish.
3. Otherwise, the last successfully invoked repeatable command starts again.
4. With no applicable state, the input is a safe no-op.

An active command owns the meaning of confirmation at each step. LINE currently
has no keyboard default point, so confirmation preserves its current prompt.

## Cancellation

Escape is idempotent and follows this priority:

1. Cancel a transient selection drag.
2. Cancel the active command and its current step data.
3. Clear the typed command buffer.
4. Clear the selection set.
5. Remain idle.

Undo and redo safely cancel active LINE input and remove stale selection IDs after
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

Left-to-right drag uses `SelectionService::containedWindow`: only completely
contained entities are included. Right-to-left drag uses
`SelectionService::crossingWindow`: contained and intersecting entities are
included. Shift+drag removes matching entities. The controller chooses the query;
the widget only supplies drag points. Clicks below the screen drag threshold remain
point picks.

The selection model is deliberately independent of command state. Future editing
commands can consume a preselection, or enter a Select objects state and use the
same click/window operations before Enter/Space confirms the selection.

## Transactions and Clipboard

Delete never mutates the model from a key handler. One `DeleteEntitiesCommand`
extracts all selected supported entities as one history entry. Undo restores their
original objects and IDs; redo removes them again.

The internal CAD clipboard stores deterministic value snapshots, currently for
Line entities. It stores layer, geometry, and graphics data, never entity pointers.
Copy does not mutate the document. Cut is copy followed by one undoable multi-delete
transaction. Paste uses one `AddLinesCommand`, assigns new IDs once, and offsets
each paste generation by +100 mm in X and Y. Undo removes the complete paste and
redo restores the same pasted IDs. Future entity snapshot variants and interactive
base-point placement can extend this boundary without changing UI routing.

## Shortcut and Focus Routing

The application routes Ctrl+Z, Ctrl+Y, Ctrl+C, Ctrl+X, Ctrl+V, Ctrl+A, Delete,
Escape, drafting F-keys, and direct command typing to the shared controller when
drafting owns input.

Actual text editors take priority. When focus is in `QLineEdit`, `QTextEdit`,
`QPlainTextEdit`, or an editable combo box, Qt native text input and editing
shortcuts are not intercepted. Clicking the canvas restores drafting focus.

Right-click policy for Phase 1.0.1 is deterministic: during an active command it
acts as Enter/confirm; while idle it repeats the last repeatable command. Right
drag is reserved and does not create selection windows.

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

Dynamic text, command prompts, selection/crossing rectangles, snap markers,
crosshair, LINE preview, and future tracking/preview graphics are interaction or
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
