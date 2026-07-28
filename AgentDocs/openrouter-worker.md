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
are in `tools/openrouter_worker/README.md`. The rationale and behavior of the
tightened delegation loop are recorded in
`RomhackDocs/openrouter-worker-loop.md`.

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
2. Delegate one narrow mechanical unit at a time, normally naming two or three
   exact files and the required function signatures.
3. Inspect architecture and repository conventions first, then provide the
   relevant facts, exact symbol locations, existing helper names, and examples in
   the prompt.
4. Use the default inspection-only command allowlist. Grant a build or test
   command only when it is cheap, targeted, and materially useful.
5. Inspect the autonomous worktree, actual files, full diff, and reported checks.
6. Reject any result whose file-scope check failed.
7. Run small representative tests first, then builds and broader tests
   independently in the primary worktree.
8. Complete tiny omissions directly when another delegation would cost more than
   the edit and review.
9. Remove the temporary worktree after accepting or rejecting the changes.

Autonomous worktrees start from committed `HEAD`; staged, unstaged, and untracked
files from the active worktree are absent. Commit specifications or source changes
that the worker needs before delegating. The server rejects autonomous delegation
when the active worktree is dirty so missing prerequisites fail fast instead of
consuming a model run.

Do not combine a cross-layer feature into one autonomous request merely because
all edits share one policy. Split pure resolver/API work, integration hooks, and
tests into separate requests when each chunk can be reviewed independently. In
practice, a five-file resolver-plus-hook-plus-tests request exhausted a 24-step
budget after only its header edits; a larger retry completed, but consumed nearly
all 40 steps. Smaller two-to-three-file chunks are easier to supervise, cheaper to
retry, and better aligned with the worker's early-edit limit.

Successful and timed-out runs remain under
`/tmp/pokemonromhack-opencode/` for review. They are never merged automatically.
Other failed runs are cleaned up automatically.

Do not run concurrent autonomous tasks with overlapping files. Parallel work is
appropriate only when file ownership and integration boundaries are disjoint.

## Current execution limits

- Default wall-clock timeout: 8 minutes.
- Default maximum agent steps: 16.
- The worker must make its first edit within six inspection calls or stop with a
  blocker instead of consuming the full budget on exploration.
- Autonomous calls require an exact one-to-four-file edit allowlist and report
  out-of-scope changes.
- Configurable agent-step range: 8–64.
- Default shell access: Git inspection and `rg` only.
- Full builds and full test suites remain with Codex.
- A timeout retains and reports the partial worktree and diff.
- Results report observed agent steps and token usage when OpenCode emits it.

Treat a run that reaches its step ceiling as incomplete even when OpenCode exits
successfully. Check the worker summary, Git status, and diff against every
acceptance criterion before integration. Remove incomplete worktrees and delegate
the missing bounded chunk again; do not infer completion from exit code alone.

## Token-efficiency policy

Delegation is intended to reduce primary-model effort, not merely move work to
another model. Account for the worker prompt, repeated repository context,
failed/retried runs, primary review, integration, and validation.

Use autonomous delegation when all of the following are true:

- The architecture and acceptance criteria are already decided.
- All required source and specifications are committed.
- The unit is mechanical and normally spans two or three files.
- The implementation is large enough to offset delegation overhead, generally
  at least 30-50 straightforward lines.
- The result has an independently reviewable boundary such as resolver/API code,
  one call-site hook, or focused tests.

Keep the work with Codex when it is architecture-heavy, ambiguous, security
sensitive, a tiny correction, or inseparable from primary review. Start routine
tasks at the 16-step default. Increase the budget only after narrowing the task
and identifying why 16 steps are insufficient.

Observed results motivating this policy:

- A five-file cross-layer request used 24 steps and produced only header edits.
- Retrying that broad request used 39 of 40 steps.
- A request launched before its prerequisite commit used 24 steps and produced
  no edits.
- The same mechanical routing task, on a committed base and limited to three
  files, completed in 10 steps.

Reported token totals include provider/cache accounting and are not direct billing
figures, but broad retries processed substantially more context than the accepted
small run. Review can also erase apparent savings: one later two-file run produced
a malformed test edit that primary review had to correct.

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

## Local configuration

The MCP registration is intentionally local and ignored by Git. A fresh clone,
workspace recreation, or removal of previously tracked `.codex/config.toml` can
leave OpenCode installed while making the worker disappear from Codex tool
discovery. Preserve or migrate the local registration before untracking editor
configuration, then relaunch Codex.

The expected repository-local configuration is:

```toml
[mcp_servers.openrouter_worker]
command = "python3"
args = ["/absolute/path/to/PokemonRomhack/tools/openrouter_worker/server.py"]
env_vars = ["OPENROUTER_API_KEY", "OPENROUTER_MODEL", "OPENROUTER_MAX_TOKENS"]
startup_timeout_sec = 10
tool_timeout_sec = 1860
```

Use an absolute server path so startup does not depend on the editor's working
directory. If `opencode` is installed through NVM but absent from Codex's `PATH`,
the worker searches `~/.nvm/versions/node/*/bin/opencode`; this is not itself an
installation failure. Diagnose registration with `codex mcp list` before
reinstalling OpenCode.

## Task template

```text
Task: Implement <one mechanical unit>.
Files: Restrict edits to <two or three exact paths>.
Signatures: Add or modify <exact function signatures>.
Context: <architecture already decided by Codex>.
Acceptance: <specific observable behavior>.
Do not: Redesign adjacent systems, run a full build, commit, or push.
Allowed commands: Use the default inspection-only allowlist.
```

Delegate only when the remaining mechanical implementation is large enough to
offset specification and review overhead. Tiny edits and architecture-heavy work
are usually cheaper to perform directly.

After integration, use the exact repository test filename when selecting a test
file:

```sh
make -j4 check TESTS=test/randomizer.c
```

`TESTS=randomizer` is interpreted as a test-name prefix, not a filename filter,
and can finish with `No tests found` after building the test binary.
