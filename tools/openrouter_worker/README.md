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

The worker can read, search, and edit inside that isolated worktree. Shell access is
default-deny. Its defaults allow only Git inspection, `rg`, `make`, and Python
unittest commands. Callers may provide up to 16 additional command patterns, which
are validated before being added. Network tools, nested agents, external-directory
access, commits, pushes, and interactive questions are disabled.

OpenCode runs in fast headless mode with project configuration, external skills,
default plugins, model-catalog refreshes, LSP downloads, and file watching disabled.
The generated worker configuration is passed in-memory rather than written into the
worktree. Failed or timed-out runs include partial OpenCode debug output and remove
their temporary worktree automatically.

The tool returns the OpenCode summary, worktree path, Git status, diff stat, full
diff (subject to an output cap), and stderr. Codex must inspect the resulting changes
before integrating them. Worktrees are intentionally retained until review is
complete; remove one afterward with:

```sh
git worktree remove /tmp/pokemonromhack-opencode/task-...
```

Only committed `HEAD` content is present in an autonomous worktree. Staged,
unstaged, and untracked files from the active worktree are not included. Commit task
specifications and source changes that the worker must read; never copy credentials,
saves, or generated ROMs into it.

## Verification

Run the dependency-free unit tests:

```sh
python3 -m unittest tools/openrouter_worker/test_server.py
```

An end-to-end request requires `OPENROUTER_API_KEY` and network access. After
restarting Codex, ask it to list or use the `openrouter_worker` tools.
