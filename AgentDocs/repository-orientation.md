# Repository Orientation for Future Agents

This is the short entry point for work in this repository. It summarizes the
maintained documentation but does not replace feature-specific plans or
handoffs.

## What This Repository Is

This is NiftyGrimoire's romhack fork of RHH's `pokeemerald-expansion`, based on
stable version `1.16.2`. Expansion is a GBA Pokemon engine and development base,
not a finished game by itself. This fork is building a deterministic,
per-save gameplay randomizer while retaining Emerald's map and story for now.

The expected integration branch is `romhack/main`.

## Instruction and Documentation Precedence

Use sources in this order:

1. Root `AGENTS.md` for non-negotiable repository and delegation rules.
2. The relevant `AgentDocs/` implementation handoff for current state and the
   next safe unit of work.
3. The corresponding `AgentDocs/hack-plans/` document for authoritative design
   decisions and intended scope.
4. `RomhackDocs/` for detailed descriptions of implemented romhack systems and
   historical engineering rationale.
5. Root and `docs/` documentation for upstream Expansion behavior, conventions,
   installation, features, and tutorials.

When a status handoff and an older broad plan disagree about completed work, use
the handoff for current implementation status and the plan for unresolved design
intent. Confirm both against the source before editing.

## Git Safety

- `origin` is the trusted NiftyGrimoire fork and is the only permitted push
  destination.
- `upstream` is the original RHH repository. It is read/fetch/compare-only unless
  the user deliberately requests an upstream integration.
- Inspect the branch, worktree, and remotes before committing or pushing.
- Preserve unrelated tracked and untracked user changes.
- Do not commit generated ROMs, save files, credentials, or other private data.
- A request to edit or implement does not by itself authorize committing,
  merging, or pushing.

## Repository Map

- `include/config/`: compile-time Expansion and romhack feature gates.
- `include/`: public declarations, shared structures, and constants.
- `src/`: C implementation; large static datasets commonly live under
  `src/data/`.
- `data/`: maps, scripts, layouts, text, tilesets, and other game content.
- `graphics/` and `sound/`: source assets.
- `test/`: unit and battle tests.
- `docs/`: upstream Expansion reference, style guide, install guides, and
  tutorials.
- `AgentDocs/`: agent-facing plans, current handoffs, and operating guidance.
- `RomhackDocs/`: detailed documentation of implemented fork-specific systems.
- `tools/`: build helpers and local development tooling.

Use `rg` to locate existing helpers and call sites before adding a new API.
Prefer established Expansion data accessors and configuration patterns over
parallel mechanisms.

## Build, Test, and Style

Common validation commands are:

```sh
make -j$(nproc)
make check -j
make check TESTS="<focused test name>"
make pokeemerald-test.elf TESTS="<focused test name>"
```

To select one test source file, pass its exact repository-relative filename,
including `.c`, for example:

```sh
make -j4 check TESTS=test/randomizer.c
```

A bare value such as `TESTS=randomizer` is treated as a test-name prefix and does
not select `test/randomizer.c`.

Run the smallest relevant checks first, then a normal ROM build and broader
tests in proportion to the change. Overworld behavior often cannot be exercised
by the automated battle harness, so pair pure resolver tests with source review
and targeted manual emulator checks when required.

Follow `docs/STYLEGUIDE.md`:

- Functions and structs use `PascalCase`.
- Variables and fields use `camelCase`.
- Globals use `g`; file-static symbols use `s`.
- Macros and constants use `CAPS_WITH_UNDERSCORES`.
- C and header files use four spaces; assembly and script files use tabs.
- Comments should explain why, not restate the code.
- Keep changes minimally invasive and configurable where appropriate.

The complete engine feature inventory is in `FEATURES.md`; installation and
toolchain guidance is in `INSTALL.md`; focused implementation tutorials are
indexed by `docs/SUMMARY.md`.

## Current Romhack Architecture

The randomizer is runtime-resolved and deterministic per save:

- Each new save stores a nonzero `u32` seed and algorithm version in
  `SaveBlock3`.
- `RandomizerHash` is stateless and category-separated. Randomizer lookups must
  not read or advance either mutable game RNG.
- Callers must pass stable identity explicitly, such as map, encounter method,
  original species, and slot.
- General species eligibility excludes disabled, invalid, battle-only, and
  context-dependent special forms. Feature-specific pools narrow it further.
- Hash categories and behavior are save-format API; incompatible changes require
  an algorithm-version change.

Foundation, ordinary wild encounter randomization, and BST-scaled encounter
pools are implemented. The encounter system preserves vanilla encounter checks,
method, weighted slot selection, and level, then replaces only the species.
Ordinary pools exclude special classifications reserved for the dedicated
Legendary scripted pool. Ordinary `setwildbattle` encounters use the regular
BST-scaled resolver with their fixed scripted level. DexNav, visible overworld
encounters, roamers, and Battle Pike/Pyramid paths remain outside the completed
ordinary-encounter scope.

Before any randomizer work, read both:

- `AgentDocs/randomizer-implementation.md`
- `AgentDocs/hack-plans/gameplay-randomizer-overview.md`

For encounter internals, also read
`RomhackDocs/wild-encounter-randomization.md`.

The documented next phase is scripted Legendary encounters. Starter
randomization, abilities, learnsets, progression rules, evolution accessibility,
world-item randomization, and quality-of-life items remain future work with
unresolved policy recorded in the plan and handoff. Do not advertise a config
gate or behavior as implemented until its code and validation exist.

## Delegation

The optional OpenRouter worker is for bounded, mechanical work that is likely to
save primary-agent effort. Architecture, ambiguous policy, review, testing, and
integration stay with the primary agent. Before using it, read
`AgentDocs/openrouter-worker.md`; setup details are in
`tools/openrouter_worker/README.md`.

Worker tasks must use isolated worktrees, exact file allowlists, and observable
acceptance criteria. Never send secrets, saves, generated ROMs, private data, or
unrelated repository content. Workers may not commit, merge, or push, and their
diffs must be reviewed and tested independently.

## Keeping This Summary Useful

Update this file only when repository-wide workflow, architecture, layout, or
major implementation status changes. Put detailed design decisions in the
authoritative hack plan, current execution state in the implementation handoff,
and subsystem internals in `RomhackDocs/`.
