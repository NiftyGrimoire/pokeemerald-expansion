"""OpenCode-backed autonomous worker for isolated implementation tasks."""

from __future__ import annotations

import json
import os
import re
import shutil
import subprocess
import tempfile
import time
import uuid
from pathlib import Path
from typing import Any


DEFAULT_MODEL = "minimax/minimax-m3"
DEFAULT_TIMEOUT_SECONDS = 480
MAX_TIMEOUT_SECONDS = 1800
DEFAULT_MAX_STEPS = 32
MIN_MAX_STEPS = 8
MAX_MAX_STEPS = 64
MAX_RESULT_BYTES = 160_000
WORKTREE_PARENT = Path(tempfile.gettempdir()) / "pokemonromhack-opencode"
SAFE_COMMAND_PATTERN = re.compile(r"^[A-Za-z0-9_./*+=:, -]+$")

DEFAULT_ALLOWED_COMMANDS = (
    "git status*",
    "git diff*",
    "git log*",
    "git show*",
    "rg *",
)


class AutonomousWorkerError(Exception):
    pass


class AutonomousWorkerTimeout(AutonomousWorkerError):
    def __init__(self, message: str, stdout: str, stderr: str):
        super().__init__(message)
        self.stdout = stdout
        self.stderr = stderr


def _run(
    args: list[str],
    *,
    cwd: Path,
    env: dict[str, str] | None = None,
    timeout: int = 60,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    try:
        result = subprocess.run(
            args,
            cwd=cwd,
            env=env,
            stdin=subprocess.DEVNULL,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        stdout = exc.stdout or ""
        stderr = exc.stderr or ""
        if isinstance(stdout, bytes):
            stdout = stdout.decode("utf-8", errors="replace")
        if isinstance(stderr, bytes):
            stderr = stderr.decode("utf-8", errors="replace")
        detail = "\n".join(
            part for part in (stdout.strip(), stderr.strip()) if part
        )
        raise AutonomousWorkerTimeout(
            f"Command timed out after {timeout} seconds: {args[0]}"
            + (f"\nPartial output:\n{detail[-6000:]}" if detail else ""),
            stdout,
            stderr,
        ) from exc
    if check and result.returncode != 0:
        detail = (result.stderr or result.stdout).strip()
        raise AutonomousWorkerError(
            f"{args[0]} failed with exit code {result.returncode}: {detail[-2000:]}"
        )
    return result


def _bounded_timeout(value: Any) -> int:
    if value is None:
        return DEFAULT_TIMEOUT_SECONDS
    if not isinstance(value, int) or isinstance(value, bool):
        raise AutonomousWorkerError("timeout_seconds must be an integer.")
    if not 60 <= value <= MAX_TIMEOUT_SECONDS:
        raise AutonomousWorkerError(
            f"timeout_seconds must be between 60 and {MAX_TIMEOUT_SECONDS}."
        )
    return value


def _allowed_commands(value: Any) -> list[str]:
    if value is None:
        return list(DEFAULT_ALLOWED_COMMANDS)
    if (
        not isinstance(value, list)
        or len(value) > 16
        or not all(isinstance(item, str) and item.strip() for item in value)
    ):
        raise AutonomousWorkerError(
            "allowed_commands must contain between 1 and 16 command patterns."
        )
    commands = []
    for command in value:
        command = command.strip()
        if len(command) > 120 or not SAFE_COMMAND_PATTERN.fullmatch(command):
            raise AutonomousWorkerError(f"Unsafe command pattern: {command!r}")
        commands.append(command)
    return commands


def _bounded_max_steps(value: Any) -> int:
    if value is None:
        return DEFAULT_MAX_STEPS
    if not isinstance(value, int) or isinstance(value, bool):
        raise AutonomousWorkerError("max_steps must be an integer.")
    if not MIN_MAX_STEPS <= value <= MAX_MAX_STEPS:
        raise AutonomousWorkerError(
            f"max_steps must be between {MIN_MAX_STEPS} and {MAX_MAX_STEPS}."
        )
    return value


def _opencode_config(
    model: str, allowed_commands: list[str], max_steps: int = DEFAULT_MAX_STEPS
) -> dict[str, Any]:
    bash_permissions = {"*": "deny"}
    bash_permissions.update({command: "allow" for command in allowed_commands})
    return {
        "$schema": "https://opencode.ai/config.json",
        "share": "disabled",
        "model": f"openrouter/{model}",
        "provider": {"openrouter": {"models": {model: {}}}},
        "agent": {
            "codex-worker": {
                "description": "Bounded implementation worker supervised by Codex",
                "mode": "primary",
                "temperature": 0.2,
                "steps": max_steps,
                "permission": {
                    "*": "deny",
                    "read": "allow",
                    "glob": "allow",
                    "grep": "allow",
                    "list": "allow",
                    "edit": "allow",
                    "bash": bash_permissions,
                    "external_directory": "deny",
                    "task": "deny",
                    "webfetch": "deny",
                    "websearch": "deny",
                    "question": "deny",
                    "doom_loop": "deny",
                },
            }
        },
    }


def _extract_text_events(raw_output: str) -> str:
    text_parts: list[str] = []
    for line in raw_output.splitlines():
        try:
            event = json.loads(line)
        except json.JSONDecodeError:
            continue
        part = event.get("part", {})
        if part.get("type") == "text" and isinstance(part.get("text"), str):
            text_parts.append(part["text"])
    return "\n".join(text_parts).strip()


def _truncate(text: str, limit: int = MAX_RESULT_BYTES) -> str:
    data = text.encode("utf-8", errors="replace")
    if len(data) <= limit:
        return text
    return data[:limit].decode("utf-8", errors="ignore") + "\n...[truncated]"


def _find_opencode() -> str | None:
    configured = os.environ.get("OPENCODE_BIN")
    if configured:
        candidate = Path(configured).expanduser()
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return str(candidate)
        raise AutonomousWorkerError(
            f"OPENCODE_BIN is not an executable file: {candidate}"
        )

    discovered = shutil.which("opencode")
    if discovered:
        return discovered

    nvm_root = Path.home() / ".nvm" / "versions" / "node"
    candidates = sorted(
        nvm_root.glob("*/bin/opencode"),
        key=lambda path: path.stat().st_mtime,
        reverse=True,
    )
    return str(candidates[0]) if candidates else None


def run_task(repo_root: Path, arguments: dict[str, Any]) -> str:
    if not os.environ.get("OPENROUTER_API_KEY"):
        raise AutonomousWorkerError(
            "OPENROUTER_API_KEY is not set in the MCP server environment."
        )
    opencode = _find_opencode()
    if opencode is None:
        raise AutonomousWorkerError(
            "OpenCode is not installed. Install it with: npm install -g opencode-ai"
        )

    task = arguments.get("task")
    if not isinstance(task, str) or not task.strip():
        raise AutonomousWorkerError("task must be a non-empty string.")
    context = arguments.get("context", "")
    if not isinstance(context, str):
        raise AutonomousWorkerError("context must be a string.")

    timeout = _bounded_timeout(arguments.get("timeout_seconds"))
    max_steps = _bounded_max_steps(arguments.get("max_steps"))
    commands = _allowed_commands(arguments.get("allowed_commands"))
    model = os.environ.get("OPENROUTER_MODEL", DEFAULT_MODEL)

    WORKTREE_PARENT.mkdir(mode=0o700, parents=True, exist_ok=True)
    worktree = WORKTREE_PARENT / (
        f"task-{time.strftime('%Y%m%d-%H%M%S')}-{uuid.uuid4().hex[:8]}"
    )
    _run(
        ["git", "worktree", "add", "--detach", str(worktree), "HEAD"],
        cwd=repo_root,
    )

    try:
        return _run_in_worktree(
            worktree,
            task.strip(),
            context.strip(),
            model,
            commands,
            timeout,
            max_steps,
            opencode,
        )
    except AutonomousWorkerError as exc:
        cleanup = _run(
            ["git", "worktree", "remove", "--force", str(worktree)],
            cwd=repo_root,
            check=False,
        )
        if cleanup.returncode != 0:
            raise AutonomousWorkerError(
                f"{exc}\nFailed worktree retained at {worktree}: "
                f"{cleanup.stderr.strip()}"
            ) from exc
        raise


def _run_in_worktree(
    worktree: Path,
    task: str,
    context: str,
    model: str,
    commands: list[str],
    timeout: int,
    max_steps: int,
    opencode: str,
) -> str:
    prompt = f"""\
Implement the bounded task below in this isolated Git worktree.

TASK
{task}

CONSTRAINTS AND ACCEPTANCE CRITERIA
{context or "(none supplied)"}

SUPERVISION RULES
- A senior Codex agent owns architecture, integration, and final review.
- Follow the existing repository style and inspect relevant code before editing.
- Do not broaden scope, commit, push, install dependencies, or access the network.
- Edit only the files needed for the stated task. Stop instead of redesigning adjacent code.
- You may edit files and run only commands permitted by the harness.
- Do not run a full build or full test suite. Codex owns independent validation.
- Finish with a concise summary of changed files, checks run, failures, and risks.
"""

    env = os.environ.copy()
    env["OPENCODE_CONFIG_CONTENT"] = json.dumps(
        _opencode_config(model, commands, max_steps)
    )
    env["OPENCODE_DISABLE_CHANNEL_DB"] = "true"
    env["OPENCODE_DISABLE_PROJECT_CONFIG"] = "true"
    env["OPENCODE_DISABLE_AUTOUPDATE"] = "true"
    env["OPENCODE_DISABLE_MODELS_FETCH"] = "true"
    env["OPENCODE_DISABLE_DEFAULT_PLUGINS"] = "true"
    env["OPENCODE_DISABLE_EXTERNAL_SKILLS"] = "true"
    env["OPENCODE_DISABLE_LSP_DOWNLOAD"] = "true"
    env["OPENCODE_EXPERIMENTAL_DISABLE_FILEWATCHER"] = "true"
    env["OPENCODE_FAST_BOOT"] = "true"
    command = [
        opencode,
        "run",
        "--pure",
        "--print-logs",
        "--log-level",
        "DEBUG",
        "--format",
        "json",
        "--agent",
        "codex-worker",
        "--model",
        f"openrouter/{model}",
        "--dir",
        str(worktree),
        prompt,
    ]
    timed_out = False
    try:
        result = _run(
            command,
            cwd=worktree,
            env=env,
            timeout=timeout,
            check=False,
        )
    except AutonomousWorkerTimeout as exc:
        timed_out = True
        result = subprocess.CompletedProcess(
            command, 124, stdout=exc.stdout, stderr=exc.stderr
        )

    status = _run(
        ["git", "status", "--short"], cwd=worktree, check=False
    ).stdout.strip()
    # Make new files visible to `git diff` without staging their contents so the
    # reviewer receives one patch containing tracked and untracked changes.
    _run(
        ["git", "add", "--intent-to-add", "--", "."],
        cwd=worktree,
        check=False,
    )
    diff_stat = _run(
        ["git", "diff", "--stat"], cwd=worktree, check=False
    ).stdout.strip()
    diff = _run(
        ["git", "diff", "--no-ext-diff"], cwd=worktree, check=False
    ).stdout
    summary = _extract_text_events(result.stdout)
    stderr = result.stderr.strip()

    return _truncate(
        "\n".join(
            [
                f"OpenCode exit code: {result.returncode}",
                f"Timed out: {'yes' if timed_out else 'no'}",
                f"Model: openrouter/{model}",
                f"Maximum agent steps: {max_steps}",
                f"Isolated worktree: {worktree}",
                "",
                "WORKER SUMMARY",
                summary or "(no text summary returned)",
                "",
                "GIT STATUS",
                status or "(clean)",
                "",
                "DIFF STAT",
                diff_stat or "(no diff)",
                "",
                "DIFF",
                diff or "(no diff)",
                "",
                "STDERR",
                stderr[-4000:] if stderr else "(empty)",
                "",
                "The active user worktree was not modified. Review the isolated "
                "worktree before integrating or removing it.",
            ]
        )
    )
