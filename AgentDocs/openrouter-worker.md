# OpenRouter Worker Guidance

## Purpose

Use the `openrouter_worker` MCP server for bounded, routine implementation work
when doing so is likely to save primary-model effort. Good candidates include
repetitive C edits, boilerplate, mechanical refactors, small test additions, and
first-pass implementations with a clear specification.

Keep planning, architecture, security-sensitive work, ambiguous changes, final
integration, and review with Codex. Do not delegate trivial edits when describing
and reviewing the task would cost more than implementing it directly.

The configured model is `minimax/minimax-m3`. Setup and troubleshooting details
are in `tools/openrouter_worker/README.md`.

## Read-only delegation

The default `delegate_programming_task` worker is advisory and read-only:

1. Give it a precise task, relevant constraints, and the smallest useful list of
   repository files.
2. Treat its response as untrusted review input.
3. Inspect every proposed change before applying it.
4. Apply changes with normal Codex editing tools; never apply a returned patch
   blindly.
5. Run the repository's relevant checks after integration.

## Autonomous delegation

For a larger but still well-specified implementation,
`delegate_autonomous_task` may run MiniMax through OpenCode. It edits only an
isolated Git worktree and returns the resulting diff.

Codex must:

1. Keep architecture, scope decisions, and final integration in the primary
   session.
2. Delegate one narrow mechanical unit at a time, normally naming two to four
   exact files and the required function signatures.
3. Inspect architecture and repository conventions first, then provide the
   relevant facts and examples in the prompt.
4. Use the default inspection-only command allowlist. Grant a build or test
   command only when it is cheap, targeted, and materially useful.
5. Inspect the autonomous worktree, actual files, full diff, and reported checks.
6. Run builds and tests independently in the primary worktree.
7. Remove the temporary worktree after accepting or rejecting the changes.

Autonomous worktrees start from committed `HEAD`; staged, unstaged, and untracked
files from the active worktree are absent. Commit specifications or source changes
that the worker needs before delegating.

Successful and timed-out runs remain under
`/tmp/pokemonromhack-opencode/` for review. They are never merged automatically.
Other failed runs are cleaned up automatically.

Do not run concurrent autonomous tasks with overlapping files. Parallel work is
appropriate only when file ownership and integration boundaries are disjoint.

## Current execution limits

- Default wall-clock timeout: 8 minutes.
- Default maximum agent steps: 32.
- Configurable agent-step range: 8–64.
- Default shell access: Git inspection and `rg` only.
- Full builds and full test suites remain with Codex.
- A timeout retains and reports the partial worktree and diff.

## Authorization and data restrictions

The user has authorized MiniMax M3 through OpenRouter/OpenCode to read and edit
this repository inside isolated worktrees for smoke tests and future delegated
tasks.

This authorization excludes:

- Secrets and credentials.
- Save files and private user data.
- Generated ROMs.
- Files outside the repository.
- Commits and pushes.
- Unrelated repository content.

Task text and repository content read by the worker are sent to OpenRouter and the
selected model provider.

## Task template

```text
Task: Implement <one mechanical unit>.
Files: Restrict edits to <two to four exact paths>.
Signatures: Add or modify <exact function signatures>.
Context: <architecture already decided by Codex>.
Acceptance: <specific observable behavior>.
Do not: Redesign adjacent systems, run a full build, commit, or push.
Allowed commands: Use the default inspection-only allowlist.
```

Delegate only when the remaining mechanical implementation is large enough to
offset specification and review overhead. Tiny edits and architecture-heavy work
are usually cheaper to perform directly.
