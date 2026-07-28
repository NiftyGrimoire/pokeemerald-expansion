# OpenRouter worker for Codex

This repository-local MCP server lets Codex delegate work to a low-cost OpenRouter
model in two modes:

- A dependency-free, read-only implementation proposal.
- An autonomous OpenCode run that searches, edits, and tests in an isolated Git
  worktree, then returns its diff for Codex review.

## Setup

1. Create an OpenRouter API key.
2. Export it in the environment that launches VS Code:

   ```sh
   export OPENROUTER_API_KEY="your-key"
   code .
   ```

3. Restart Codex or open a new Codex session in this trusted repository.

4. To enable autonomous tasks, install OpenCode in WSL:

   ```sh
   npm install -g opencode-ai
   ```

The project configuration in `.codex/config.toml` starts the server automatically.
The default model is `minimax/minimax-m3`.

`.codex/` is ignored by this repository, so cloning the repository or removing a
previously tracked editor configuration does not recreate the MCP registration.
Keep the local file when untracking it, or restore the registration before
relaunching Codex. Prefer an absolute path to `server.py` in that local
configuration so server startup does not depend on the launch directory.

Optional environment variables:

```sh
export OPENROUTER_MODEL="minimax/minimax-m3"
export OPENROUTER_MAX_TOKENS="6000"
```

Do not put the API key in `.codex/config.toml`, `.env`, shell scripts, or other
tracked files. Requests send the task, constraints, and selected file contents to
OpenRouter and its chosen inference provider.

## Read-only tool

`delegate_programming_task` accepts:

- `task`: required, bounded implementation request
- `files`: optional repository-relative files to include
- `context`: optional constraints and acceptance criteria
- `max_tokens`: optional value between 256 and 12000

The server limits the number and size of files, rejects paths outside the repository,
rejects binary files, never invokes a shell, and never writes to the working tree.

## Autonomous tool

`delegate_autonomous_task` launches OpenCode with the configured OpenRouter model.
It creates a detached Git worktree below `/tmp/pokemonromhack-opencode/`; the active
user worktree and its uncommitted files are not copied or modified.

Every autonomous request must provide `files`, an exact allowlist of one to eight
repository-relative files the worker may edit. The prompt names this scope, and the
result reports a failed file-scope check if the worktree contains changes elsewhere.
Never integrate a result whose file-scope check failed.

The worker can read, search, and edit inside that isolated worktree. Shell access is
default-deny. Its defaults allow only Git inspection and `rg`; builds and tests are
left to Codex unless a caller explicitly grants one narrow command pattern. Callers
may provide up to 16 command patterns, which are validated before being added.
Network tools, nested agents, external-directory access, commits, pushes, and
interactive questions are disabled.

OpenCode runs with an eight-minute default timeout and a 24-step default agent limit.
Both can be adjusted per call within bounded limits. It also runs in fast headless
mode with project configuration, external skills, default plugins, model-catalog
refreshes, LSP downloads, and file watching disabled. The generated worker
configuration is passed in-memory rather than written into the worktree. A timed-out
run returns and retains its partial worktree and diff for review. Other failed runs
are cleaned up automatically.

The tool returns the final OpenCode summary, observed step count, reported token
usage when OpenCode supplies it, file-scope status, worktree path, Git status, diff
stat, full diff (subject to an output cap), and a short stderr tail. Codex must
inspect the resulting changes before integrating them. Worktrees are intentionally
retained until review is complete; remove one afterward with:

```sh
git worktree remove /tmp/pokemonromhack-opencode/task-...
```

Only committed `HEAD` content is present in an autonomous worktree. Staged,
unstaged, and untracked files from the active worktree are not included. Commit task
specifications and source changes that the worker must read; never copy credentials,
saves, or generated ROMs into it.

Keep autonomous tasks mechanical and small: name the exact files and function
signatures and include relevant existing helper names and locations. Normally limit
the change to two to four files. The worker is instructed to make its first edit
within six inspection calls and preserve part of its budget for checks. Use 16-24
steps for routine changes; increase the limit only for a task that demonstrably
needs more implementation work. Codex should locate integration hooks, make
architecture decisions, use small representative test cases, and run the build and
tests after reviewing the diff. Complete tiny omissions directly rather than
starting another autonomous run.

Prefer separate requests for resolver/API code, call-site hooks, and tests when a
feature crosses those boundaries. Always compare the returned diff to every
acceptance criterion: reaching the configured step limit can yield a successful
OpenCode process with an incomplete edit.

## Verification

Run the dependency-free unit tests:

```sh
python3 -m unittest tools/openrouter_worker/test_server.py
```

An end-to-end request requires `OPENROUTER_API_KEY` and network access. After
restarting Codex, ask it to list or use the `openrouter_worker` tools.
