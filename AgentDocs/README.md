# Agent Documentation

This directory contains detailed operating notes and implementation handoffs for
agents working on the romhack.

- [Repository orientation](repository-orientation.md): concise start-here summary
  of documentation precedence, repository layout, workflow, conventions, and
  current romhack architecture.
- [OpenRouter worker](openrouter-worker.md): when and how to delegate bounded work
  to MiniMax M3 through OpenCode.
- [Randomizer implementation](randomizer-implementation.md): current architecture,
  completed work, validation, and the next development phase.
- [1.1.0 changelist review](pending-changelist-review-2026-08-04.md): closed
  correctness ledger and remaining manual-release gate for commit `4ebbfa896c`.
- [Hack plans](hack-plans/): authoritative design and phase task lists, beginning
  with the [gameplay randomizer](hack-plans/gameplay-randomizer-overview.md).

Player-facing behavior belongs under [`PlayerDocs/`](../PlayerDocs/), not in
agent handoffs. The active release branch is `romhack/nuzlocke-1.1.0`; the
integration branch remains `romhack/main`.

Repository-wide non-negotiable rules remain in the root `AGENTS.md`.
