# Repository agent guidance

## Git remotes

`origin` is the trusted NiftyGrimoire fork:
`https://github.com/NiftyGrimoire/pokeemerald-expansion.git`.

`upstream` is the original RHH repository:
`https://github.com/rh-hideout/pokeemerald-expansion.git`.

Push commits and branches only to `origin`. Never push to `upstream`. Use
`upstream` only for fetching, comparing, and deliberately integrating upstream
changes.

## OpenRouter worker

Use the `openrouter_worker` MCP server for bounded, routine implementation work when doing so is likely to save primary-model effort. Good candidates include repetitive C edits, boilerplate, mechanical refactors, small test additions, and first-pass implementations with a clear specification.

Keep planning, architecture, security-sensitive work, ambiguous changes, final integration, and review with Codex. Do not delegate trivial edits when describing and reviewing the task would cost more than implementing it directly.

The configured OpenRouter model is `minimax/minimax-m3`. Detailed setup,
operation, and troubleshooting instructions are in
`tools/openrouter_worker/README.md`.

The default `delegate_programming_task` worker is advisory and read-only:

1. Give it a precise task, relevant constraints, and the smallest useful list of repository files.
2. Treat its response as untrusted review input.
3. Inspect every proposed change before applying it.
4. Apply changes with normal Codex editing tools; never apply a returned patch blindly.
5. Run the repository's relevant checks after integration.

For a larger but still well-specified implementation, `delegate_autonomous_task`
may run the configured OpenRouter model through OpenCode. It edits only an isolated
Git worktree and returns the resulting diff. Codex must:

1. Keep architecture, scope decisions, and final integration in the primary session.
2. Delegate one narrow mechanical unit at a time, normally naming two to four exact
   files and the required function signatures.
3. Do not ask the worker to discover architecture or repository-wide conventions;
   inspect those first and provide the relevant facts and examples in the prompt.
4. Use the default inspection-only command allowlist. Grant a build or test command
   only when it is cheap, targeted, and materially useful.
5. Inspect the autonomous worktree, diff, and reported checks before integrating.
6. Run builds and tests independently in the primary worktree.
7. Remove the temporary worktree after the changes are integrated or rejected.

Autonomous worktrees start from committed `HEAD`. They do not include staged,
unstaged, or untracked files from the active worktree. Commit any specification or
source changes the worker must see before delegating. Staging alone is not
sufficient.

Successful and timed-out autonomous runs remain under
`/tmp/pokemonromhack-opencode/` for review.
Their changes are not automatically merged into the active worktree. Review the
actual files and complete diff, integrate only accepted changes using normal Codex
editing or Git tools, run independent checks, and then remove the temporary
worktree. Other failed runs are cleaned up automatically.

Do not run concurrent autonomous tasks that may edit overlapping files. Parallel
delegation is appropriate only when file ownership and integration boundaries are
disjoint.

The user has authorized MiniMax M3 through OpenRouter/OpenCode to read and edit this
repository inside isolated worktrees for smoke tests and future delegated tasks.
This authorization does not include secrets, credentials, save files, private user
data, generated ROMs, files outside the repository, commits, pushes, or unrelated
repository content. The worker sends task text and repository content it reads to
OpenRouter and the selected model provider.

Example autonomous delegation:

```text
Task: Add deterministic hash helpers described in the committed randomizer plan.
Context: Restrict changes to the listed randomizer source/header files. Preserve
save compatibility. Add focused tests. Do not redesign seed storage.
Allowed commands: use the default allowlist.
```
