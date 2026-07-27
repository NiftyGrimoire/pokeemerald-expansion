#!/usr/bin/env python3
"""Small, dependency-free MCP server for reviewable OpenRouter delegation."""

from __future__ import annotations

import json
import os
import sys
import urllib.error
import urllib.request
from pathlib import Path
from typing import Any

import autonomous


SERVER_NAME = "openrouter-worker"
SERVER_VERSION = "0.2.0"
DEFAULT_MODEL = "minimax/minimax-m3"
DEFAULT_MAX_TOKENS = 6000
MAX_MAX_TOKENS = 12000
MAX_FILES = 24
MAX_FILE_BYTES = 80_000
MAX_CONTEXT_BYTES = 240_000
REPO_ROOT = Path(__file__).resolve().parents[2]
OPENROUTER_URL = "https://openrouter.ai/api/v1/chat/completions"

SYSTEM_PROMPT = """\
You are a cost-efficient implementation worker inside a supervised coding workflow.
A senior Codex agent owns architecture, integration, and final review.

Produce a conservative implementation proposal for the task using only the supplied
context. Do not claim to have edited files or run commands. Preserve existing style
and avoid unrelated changes.

Return:
1. A short implementation summary.
2. A unified diff in a fenced diff block, using repository-relative paths.
3. Tests or checks the reviewer should run.
4. Assumptions, risks, or missing context.

If the task cannot be completed safely from the supplied context, do not invent code.
Explain exactly which files or facts are missing instead.
"""


class WorkerError(Exception):
    pass


def _error_message(exc: BaseException) -> str:
    if isinstance(exc, urllib.error.HTTPError):
        try:
            payload = exc.read().decode("utf-8", errors="replace")
            parsed = json.loads(payload)
            detail = parsed.get("error", {}).get("message", payload)
        except Exception:
            detail = str(exc)
        return f"OpenRouter returned HTTP {exc.code}: {detail}"
    if isinstance(exc, urllib.error.URLError):
        return f"Could not reach OpenRouter: {exc.reason}"
    return str(exc)


def _read_repo_files(paths: list[str]) -> str:
    if len(paths) > MAX_FILES:
        raise WorkerError(f"At most {MAX_FILES} files may be supplied per task.")

    chunks: list[str] = []
    total = 0
    for raw_path in paths:
        candidate = (REPO_ROOT / raw_path).resolve()
        try:
            relative = candidate.relative_to(REPO_ROOT)
        except ValueError as exc:
            raise WorkerError(f"Path escapes repository: {raw_path}") from exc
        if not candidate.is_file():
            raise WorkerError(f"Not a repository file: {raw_path}")

        data = candidate.read_bytes()
        if len(data) > MAX_FILE_BYTES:
            raise WorkerError(
                f"File is too large to delegate ({len(data)} bytes): {relative}"
            )
        if b"\0" in data:
            raise WorkerError(f"Binary files cannot be delegated: {relative}")

        text = data.decode("utf-8", errors="replace")
        chunk = f"\n--- FILE: {relative.as_posix()} ---\n{text}\n"
        total += len(chunk.encode("utf-8"))
        if total > MAX_CONTEXT_BYTES:
            raise WorkerError(
                f"Selected file context exceeds {MAX_CONTEXT_BYTES} bytes."
            )
        chunks.append(chunk)
    return "".join(chunks)


def _bounded_max_tokens(requested: Any) -> int:
    configured = os.environ.get("OPENROUTER_MAX_TOKENS", str(DEFAULT_MAX_TOKENS))
    try:
        value = int(requested if requested is not None else configured)
    except (TypeError, ValueError) as exc:
        raise WorkerError("max_tokens must be an integer.") from exc
    if not 256 <= value <= MAX_MAX_TOKENS:
        raise WorkerError(f"max_tokens must be between 256 and {MAX_MAX_TOKENS}.")
    return value


def delegate(arguments: dict[str, Any]) -> str:
    api_key = os.environ.get("OPENROUTER_API_KEY")
    if not api_key:
        raise WorkerError(
            "OPENROUTER_API_KEY is not set. Export it before starting VS Code/Codex."
        )

    task = arguments.get("task")
    if not isinstance(task, str) or not task.strip():
        raise WorkerError("task must be a non-empty string.")
    context = arguments.get("context", "")
    if not isinstance(context, str):
        raise WorkerError("context must be a string.")
    files = arguments.get("files", [])
    if not isinstance(files, list) or not all(isinstance(item, str) for item in files):
        raise WorkerError("files must be an array of repository-relative paths.")

    file_context = _read_repo_files(files)
    user_prompt = (
        f"TASK\n{task.strip()}\n\n"
        f"CONSTRAINTS AND CONTEXT\n{context.strip() or '(none supplied)'}\n"
        f"{file_context or '\n(no repository files supplied)'}"
    )
    model = os.environ.get("OPENROUTER_MODEL", DEFAULT_MODEL)
    body = json.dumps(
        {
            "model": model,
            "messages": [
                {"role": "system", "content": SYSTEM_PROMPT},
                {"role": "user", "content": user_prompt},
            ],
            "temperature": 0.2,
            "max_tokens": _bounded_max_tokens(arguments.get("max_tokens")),
        }
    ).encode("utf-8")
    request = urllib.request.Request(
        OPENROUTER_URL,
        data=body,
        method="POST",
        headers={
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json",
            "X-OpenRouter-Title": "PokemonRomhack Codex Worker",
        },
    )

    try:
        with urllib.request.urlopen(request, timeout=165) as response:
            result = json.load(response)
        content = result["choices"][0]["message"]["content"]
    except (KeyError, IndexError, TypeError, json.JSONDecodeError) as exc:
        raise WorkerError("OpenRouter returned an unexpected response shape.") from exc
    except (urllib.error.HTTPError, urllib.error.URLError, TimeoutError) as exc:
        raise WorkerError(_error_message(exc)) from exc

    if not isinstance(content, str) or not content.strip():
        raise WorkerError("OpenRouter returned an empty response.")

    usage = result.get("usage", {})
    usage_line = (
        f"\n\nWorker metadata: model={result.get('model', model)}, "
        f"prompt_tokens={usage.get('prompt_tokens', 'unknown')}, "
        f"completion_tokens={usage.get('completion_tokens', 'unknown')}."
    )
    return content.strip() + usage_line


READ_ONLY_TOOL = {
    "name": "delegate_programming_task",
    "description": (
        "Ask a low-cost OpenRouter coding model for a reviewable implementation "
        "proposal. It reads only explicitly listed repo files and never edits files."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "task": {
                "type": "string",
                "description": "Precise, bounded implementation task.",
            },
            "files": {
                "type": "array",
                "items": {"type": "string"},
                "description": "Repository-relative source files to include.",
                "maxItems": MAX_FILES,
                "default": [],
            },
            "context": {
                "type": "string",
                "description": "Constraints, acceptance criteria, and relevant facts.",
                "default": "",
            },
            "max_tokens": {
                "type": "integer",
                "minimum": 256,
                "maximum": MAX_MAX_TOKENS,
                "description": "Maximum completion tokens for this task.",
            },
        },
        "required": ["task"],
        "additionalProperties": False,
    },
}

AUTONOMOUS_TOOL = {
    "name": "delegate_autonomous_task",
    "description": (
        "Run an OpenRouter model through OpenCode in an isolated Git worktree. "
        "The worker may search, edit, and run allowlisted checks. It never modifies "
        "the active worktree and returns its worktree path and diff for review."
    ),
    "inputSchema": {
        "type": "object",
        "properties": {
            "task": {
                "type": "string",
                "description": "Precise, bounded implementation task.",
            },
            "context": {
                "type": "string",
                "description": "Constraints, acceptance criteria, and relevant facts.",
                "default": "",
            },
            "allowed_commands": {
                "type": "array",
                "items": {"type": "string"},
                "maxItems": 16,
                "description": (
                    "Optional OpenCode bash permission patterns. Safe defaults allow "
                    "Git inspection, rg, make, and Python unittest."
                ),
            },
            "timeout_seconds": {
                "type": "integer",
                "minimum": 60,
                "maximum": autonomous.MAX_TIMEOUT_SECONDS,
                "default": autonomous.DEFAULT_TIMEOUT_SECONDS,
            },
        },
        "required": ["task"],
        "additionalProperties": False,
    },
}

TOOLS = [READ_ONLY_TOOL, AUTONOMOUS_TOOL]


def _result(request_id: Any, result: dict[str, Any]) -> dict[str, Any]:
    return {"jsonrpc": "2.0", "id": request_id, "result": result}


def _rpc_error(request_id: Any, code: int, message: str) -> dict[str, Any]:
    return {
        "jsonrpc": "2.0",
        "id": request_id,
        "error": {"code": code, "message": message},
    }


def handle(message: dict[str, Any]) -> dict[str, Any] | None:
    method = message.get("method")
    request_id = message.get("id")
    if method == "initialize":
        version = message.get("params", {}).get("protocolVersion", "2025-03-26")
        return _result(
            request_id,
            {
                "protocolVersion": version,
                "capabilities": {"tools": {"listChanged": False}},
                "serverInfo": {"name": SERVER_NAME, "version": SERVER_VERSION},
            },
        )
    if method == "notifications/initialized":
        return None
    if method == "ping":
        return _result(request_id, {})
    if method == "tools/list":
        return _result(request_id, {"tools": TOOLS})
    if method == "tools/call":
        params = message.get("params", {})
        tool_name = params.get("name")
        if tool_name not in {tool["name"] for tool in TOOLS}:
            return _rpc_error(request_id, -32602, "Unknown tool.")
        try:
            if tool_name == READ_ONLY_TOOL["name"]:
                text = delegate(params.get("arguments", {}))
            else:
                text = autonomous.run_task(
                    REPO_ROOT, params.get("arguments", {})
                )
            return _result(
                request_id,
                {"content": [{"type": "text", "text": text}], "isError": False},
            )
        except (WorkerError, autonomous.AutonomousWorkerError) as exc:
            return _result(
                request_id,
                {"content": [{"type": "text", "text": str(exc)}], "isError": True},
            )
    if request_id is None:
        return None
    return _rpc_error(request_id, -32601, f"Method not found: {method}")


def main() -> None:
    for line in sys.stdin:
        if not line.strip():
            continue
        try:
            message = json.loads(line)
            response = handle(message)
        except json.JSONDecodeError:
            response = _rpc_error(None, -32700, "Parse error.")
        except Exception as exc:
            print(f"{SERVER_NAME}: {exc}", file=sys.stderr, flush=True)
            response = _rpc_error(None, -32603, "Internal error.")
        if response is not None:
            print(json.dumps(response, separators=(",", ":")), flush=True)


if __name__ == "__main__":
    main()
