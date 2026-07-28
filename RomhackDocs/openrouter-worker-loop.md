# OpenRouter Worker Delegation Loop

## Purpose

The repository-local OpenRouter worker delegates bounded mechanical work to
MiniMax through OpenCode while Codex retains architecture, review, integration,
and validation.

The autonomous worker edits a detached Git worktree under
`/tmp/pokemonromhack-opencode/`. It cannot commit, push, access the network,
launch nested agents, or modify the active user worktree.

## Why the Loop Was Tightened

The BST encounter implementation exposed several workflow costs:

- One 32-step worker run exhausted its budget while exploring and made no edits.
- A later 48-step run implemented only part of its assigned call sites.
- Returned progress narration and debug logs consumed review context.
- Codex had to detect an existing helper-name collision during compilation.
- An exhaustive worker-authored test timed out on the GBA test runner.
- The harness did not verify that edits stayed within the files named by Codex.

The final BST implementation was correct after Codex review, but the delegation
loop did not demonstrate a clear primary-model token saving.

## Current Autonomous Contract

Every `delegate_autonomous_task` request must provide:

- One bounded mechanical task.
- An exact allowlist of one to eight repository-relative files.
- Existing helper names and symbol locations when known.
- Observable acceptance criteria.
- A step limit appropriate to the task.

The default limit is 24 OpenCode steps. The worker is instructed to make its
first edit within six inspection calls or stop and report a blocker. At least
one quarter of its budget should remain for editing and focused checks.

The generated prompt prohibits adjacent redesign, full builds, commits, pushes,
and network access. Shell access remains default-deny except for explicitly
allowlisted inspection or targeted-check commands.

## Scope Validation

After OpenCode exits, the harness collects tracked, staged, and untracked
changes and compares them with the request's file allowlist.

The result reports either:

```text
File scope: PASSED
```

or a list of out-of-scope paths. Codex must reject a failed scope result. The
worktree remains available for inspection, but its patch must not be integrated.

This is a review safeguard rather than automatic patch application. Codex still
inspects the actual worktree and full diff before applying any accepted change.

## Result Telemetry

Autonomous results include:

- Configured model and maximum step limit.
- Observed OpenCode step count parsed from worker logs.
- Input/output token data when OpenCode emits compatible usage events.
- File-scope status.
- Final worker summary.
- Git status, diff statistics, and the reviewable diff.
- A bounded stderr tail.

Only the final text summary is returned instead of concatenating intermediate
planning messages. The overall result cap is 80 KB, reduced from 160 KB.

Token telemetry may still report `unavailable` when the installed OpenCode
version does not emit usage data in its JSON event stream. It measures worker
usage only; it cannot calculate the counterfactual tokens Codex would have used
without delegation.

## Recommended Workflow

1. Codex locates the exact integration points and existing helpers.
2. Split work by non-overlapping file ownership and observable behavior.
3. Use 16-24 steps for routine mechanical changes.
4. Increase the limit only when the implementation itself requires more work.
5. Review scope status, actual files, and the complete diff.
6. Reject out-of-scope or architecture-changing results.
7. Apply accepted edits under Codex supervision.
8. Run a small representative test before broader suites and builds.
9. Fix tiny omissions directly when another delegation would cost more.
10. Remove the temporary worktree.

Automatic merging and automatic patch acceptance remain intentionally
unsupported.

## Validation

`python3 -m unittest tools/openrouter_worker/test_server.py` passes 12 tests.
Coverage includes:

- Isolated-worktree editing.
- Required safe file scopes.
- Detection of unauthorized files.
- Step and token metric extraction.
- Final-summary selection.
- Timeout retention with a partial diff.
- Default-deny command permissions.

The MCP server must be restarted after integrating harness changes before the
new autonomous tool schema is visible to Codex.
