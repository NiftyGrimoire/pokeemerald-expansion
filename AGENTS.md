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

Use the `openrouter_worker` MCP server for bounded mechanical work when delegation
is likely to save primary-model effort. Keep architecture, ambiguous decisions,
security-sensitive work, review, testing, and integration with Codex.

Before delegating or handling an existing autonomous worktree, read
`AgentDocs/openrouter-worker.md`. Detailed setup and troubleshooting are in
`tools/openrouter_worker/README.md`.

Never send secrets, credentials, save files, private data, generated ROMs, or
unrelated content to the worker. MiniMax may edit only isolated worktrees and may
not commit or push.

## Implementation handoffs

Read the relevant document under `AgentDocs/` before beginning feature work. The
current randomizer status and next-phase guidance are in
`AgentDocs/randomizer-implementation.md`.
