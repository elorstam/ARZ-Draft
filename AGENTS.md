# ARZ Draft Agent Rules

ARZ Studio CAD is developed as an agent-friendly native C++23 CAD application.

## Current Product Scope

V1 is a professional manual 2D architectural CAD application.

BIM is planned for V2.

## Locked Architecture Rules

1. Internal geometry units are millimeters.
2. Persistent objects use stable IDs.
3. Model mutations must go through explicit command APIs.
4. Core modules must not depend on Qt UI classes.
5. Geometry must not depend on application or UI modules.
6. BIM category and CAD layer are separate concepts.
7. DWG must never become the internal document format.
8. Undo / redo is part of the transaction architecture.
9. Do not bypass module boundaries for convenience.
10. Every new feature requires tests.
11. Do not mark a phase complete while tests fail.
12. Prefer small isolated changes over broad rewrites.

## Language

All source code, class names, commands, UI labels, comments,
technical documentation and tests are written in English.

Turkish localization is not required for V1.

## Agent Development Rules

Before editing code:

1. Read AGENTS.md.
2. Read docs/ARCHITECTURE.md when available.
3. Read docs/PROJECT_STATE.md when available.
4. Respect existing module boundaries.
5. Do not silently change locked architecture decisions.

For every task:

- identify the goal,
- identify allowed modules,
- implement the smallest valid change,
- add or update tests,
- build the project,
- run the complete relevant test suite,
- report remaining issues.

## Current Phase

Phase 1.1 — Rendering Foundation.

Do not start Phase 1.2, BIM, DWG, OpenCascade, AI features,
automatic sections or automatic elevations during Phase 1.1.

## End-of-Phase Workflow

Every development phase must finish in this order:

1. Implement the scoped change and tests.
2. Configure and build the supported Debug application.
3. Run the complete relevant automated suite.
4. Run `git diff --check` and inspect repository status/diff.
5. Update durable project documentation only after verification passes.
6. Commit the intended source changes and push the current branch without force.
7. Launch the normal ARZ Studio CAD application without `--smoke-test`.
8. Leave the application open for human live acceptance when the environment permits.

Automated completion and human acceptance are separate. Agents may report
AUTOMATED COMPLETE only after build, tests, diff check, commit, and push succeed.
Only the user may mark a phase HUMAN ACCEPTED after testing the live application.
