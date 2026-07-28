#!/usr/bin/env python3

import json
import os
import subprocess
import sys
import tempfile
import unittest
from io import BytesIO
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parent))
import server
import autonomous


class FakeResponse:
    def __init__(self, payload):
        self.payload = BytesIO(json.dumps(payload).encode())

    def __enter__(self):
        return self.payload

    def __exit__(self, *_args):
        return False


class ServerTests(unittest.TestCase):
    def test_initialize_and_tool_listing(self):
        initialized = server.handle(
            {
                "jsonrpc": "2.0",
                "id": 1,
                "method": "initialize",
                "params": {"protocolVersion": "2025-03-26"},
            }
        )
        self.assertEqual(initialized["result"]["protocolVersion"], "2025-03-26")
        listed = server.handle({"jsonrpc": "2.0", "id": 2, "method": "tools/list"})
        self.assertEqual(
            [tool["name"] for tool in listed["result"]["tools"]],
            ["delegate_programming_task", "delegate_autonomous_task"],
        )

    def test_rejects_path_outside_repository(self):
        with self.assertRaisesRegex(server.WorkerError, "escapes repository"):
            server._read_repo_files(["../outside.txt"])

    def test_delegate_calls_openrouter_and_reports_usage(self):
        payload = {
            "model": "minimax/minimax-m3",
            "choices": [{"message": {"content": "proposal"}}],
            "usage": {"prompt_tokens": 10, "completion_tokens": 5},
        }
        with (
            patch.dict(os.environ, {"OPENROUTER_API_KEY": "test-key"}, clear=True),
            patch("server.urllib.request.urlopen", return_value=FakeResponse(payload)),
        ):
            output = server.delegate({"task": "Make a small change."})
        self.assertIn("proposal", output)
        self.assertIn("prompt_tokens=10", output)

    def test_missing_key_is_a_clear_error(self):
        with patch.dict(os.environ, {}, clear=True):
            with self.assertRaisesRegex(server.WorkerError, "OPENROUTER_API_KEY"):
                server.delegate({"task": "Anything"})

    def test_autonomous_command_patterns_are_bounded(self):
        self.assertEqual(
            autonomous._allowed_commands(["git diff*", "make*"]),
            ["git diff*", "make*"],
        )
        with self.assertRaisesRegex(
            autonomous.AutonomousWorkerError, "Unsafe command pattern"
        ):
            autonomous._allowed_commands(["make; curl example.com"])

    def test_autonomous_config_denies_unlisted_commands(self):
        config = autonomous._opencode_config(
            "minimax/minimax-m3", ["git diff*", "make*"]
        )
        agent = config["agent"]["codex-worker"]
        permissions = agent["permission"]
        self.assertEqual(agent["steps"], autonomous.DEFAULT_MAX_STEPS)
        self.assertEqual(permissions["edit"], "allow")
        self.assertEqual(permissions["external_directory"], "deny")
        self.assertEqual(permissions["bash"]["*"], "deny")
        self.assertEqual(permissions["bash"]["make*"], "allow")

    def test_autonomous_defaults_exclude_expensive_checks(self):
        commands = autonomous._allowed_commands(None)
        self.assertNotIn("make*", commands)
        self.assertNotIn("python3 -m unittest*", commands)
        self.assertEqual(
            autonomous._bounded_max_steps(None), autonomous.DEFAULT_MAX_STEPS
        )
        self.assertEqual(autonomous.DEFAULT_MAX_STEPS, 16)
        self.assertEqual(autonomous.MAX_SCOPED_FILES, 4)
        with self.assertRaisesRegex(
            autonomous.AutonomousWorkerError, "max_steps must be between"
        ):
            autonomous._bounded_max_steps(autonomous.MAX_MAX_STEPS + 1)

    def test_autonomous_file_scope_is_required_and_repository_relative(self):
        self.assertEqual(
            autonomous._scoped_files(["src/sample.c", "src/sample.c"]),
            ["src/sample.c"],
        )
        with self.assertRaisesRegex(
            autonomous.AutonomousWorkerError, "between 1 and"
        ):
            autonomous._scoped_files(None)
        with self.assertRaisesRegex(
            autonomous.AutonomousWorkerError, "unsafe repository path"
        ):
            autonomous._scoped_files(["../outside.c"])

    def test_autonomous_rejects_dirty_worktree(self):
        with tempfile.TemporaryDirectory() as temp:
            repo = Path(temp)
            subprocess.run(["git", "init", "-q"], cwd=repo, check=True)
            (repo / "dirty.txt").write_text("uncommitted\n", encoding="utf-8")

            with self.assertRaisesRegex(
                autonomous.AutonomousWorkerError,
                "Autonomous worktrees start from committed HEAD",
            ):
                autonomous.run_task(
                    repo, {"task": "Anything", "files": ["dirty.txt"]}
                )

    def test_autonomous_summary_and_metrics_are_concise(self):
        raw_output = "\n".join(
            [
                json.dumps(
                    {"part": {"type": "text", "text": "intermediate planning"}}
                ),
                json.dumps(
                    {
                        "part": {
                            "type": "step-finish",
                            "tokens": {"input": 10, "output": 5},
                        }
                    }
                ),
                json.dumps(
                    {"part": {"type": "text", "text": "final implementation summary"}}
                ),
            ]
        )
        self.assertEqual(
            autonomous._extract_text_events(raw_output),
            "final implementation summary",
        )
        metrics = autonomous._extract_metrics(raw_output, "loop step=7")
        self.assertIn("Observed agent steps: 7", metrics)
        self.assertIn("input=10", metrics)
        self.assertIn("output=5", metrics)

    def test_finds_opencode_from_nvm_when_not_on_path(self):
        with tempfile.TemporaryDirectory() as temp:
            home = Path(temp)
            binary = home / ".nvm/versions/node/v22/bin/opencode"
            binary.parent.mkdir(parents=True)
            binary.write_text("#!/bin/sh\n", encoding="utf-8")
            binary.chmod(0o755)
            with (
                patch.object(Path, "home", return_value=home),
                patch("autonomous.shutil.which", return_value=None),
                patch.dict(os.environ, {}, clear=True),
            ):
                self.assertEqual(autonomous._find_opencode(), str(binary))

    def test_autonomous_task_edits_only_an_isolated_worktree(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            repo = root / "repo"
            bin_dir = root / "bin"
            worktrees = root / "worktrees"
            repo.mkdir()
            bin_dir.mkdir()
            (repo / "sample.txt").write_text("original\n", encoding="utf-8")
            subprocess.run(["git", "init", "-q"], cwd=repo, check=True)
            subprocess.run(
                ["git", "add", "sample.txt"], cwd=repo, check=True
            )
            subprocess.run(
                [
                    "git",
                    "-c",
                    "user.name=Test",
                    "-c",
                    "user.email=test@example.invalid",
                    "commit",
                    "-qm",
                    "initial",
                ],
                cwd=repo,
                check=True,
            )

            fake_opencode = bin_dir / "opencode"
            fake_opencode.write_text(
                """#!/bin/sh
dir=
while [ "$#" -gt 0 ]; do
    if [ "$1" = "--dir" ]; then dir=$2; shift 2; else shift; fi
done
printf 'worker edit\\n' >> "$dir/sample.txt"
printf 'new worker file\\n' > "$dir/created.txt"
printf '%s\\n' '{"part":{"type":"text","text":"fake worker complete"}}'
printf '%s\\n' 'loop step=3' >&2
""",
                encoding="utf-8",
            )
            fake_opencode.chmod(0o755)

            with (
                patch.object(autonomous, "WORKTREE_PARENT", worktrees),
                patch.dict(
                    os.environ,
                    {
                        "OPENROUTER_API_KEY": "test-key",
                        "PATH": f"{bin_dir}:{os.environ['PATH']}",
                    },
                ),
            ):
                output = autonomous.run_task(
                    repo,
                    {
                        "task": "Edit sample.",
                        "files": ["sample.txt", "created.txt"],
                    },
                )

            self.assertEqual(
                (repo / "sample.txt").read_text(encoding="utf-8"), "original\n"
            )
            self.assertIn("fake worker complete", output)
            self.assertIn("Observed agent steps: 3", output)
            self.assertIn("File scope: PASSED", output)
            self.assertIn("+worker edit", output)
            self.assertIn("+new worker file", output)
            with (
                patch.object(autonomous, "WORKTREE_PARENT", worktrees),
                patch.dict(
                    os.environ,
                    {
                        "OPENROUTER_API_KEY": "test-key",
                        "PATH": f"{bin_dir}:{os.environ['PATH']}",
                    },
                ),
            ):
                scoped_output = autonomous.run_task(
                    repo,
                    {"task": "Edit sample.", "files": ["sample.txt"]},
                )
            self.assertIn(
                "File scope: FAILED; out-of-scope changes: created.txt",
                scoped_output,
            )
            scoped_worktree_line = next(
                line
                for line in scoped_output.splitlines()
                if line.startswith("Isolated worktree: ")
            )
            scoped_worktree = Path(scoped_worktree_line.split(": ", 1)[1])
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(scoped_worktree)],
                cwd=repo,
                check=True,
            )
            worktree_line = next(
                line for line in output.splitlines()
                if line.startswith("Isolated worktree: ")
            )
            worktree = Path(worktree_line.split(": ", 1)[1])
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(worktree)],
                cwd=repo,
                check=True,
            )

    def test_timed_out_autonomous_task_retains_partial_diff(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            repo = root / "repo"
            worktrees = root / "worktrees"
            repo.mkdir()
            (repo / "sample.txt").write_text("original\n", encoding="utf-8")
            subprocess.run(["git", "init", "-q"], cwd=repo, check=True)
            subprocess.run(["git", "add", "sample.txt"], cwd=repo, check=True)
            subprocess.run(
                [
                    "git",
                    "-c",
                    "user.name=Test",
                    "-c",
                    "user.email=test@example.invalid",
                    "commit",
                    "-qm",
                    "initial",
                ],
                cwd=repo,
                check=True,
            )

            original_run = autonomous._run

            def fake_run(args, **kwargs):
                if args[0] == "/fake/opencode":
                    worktree = Path(kwargs["cwd"])
                    (worktree / "sample.txt").write_text(
                        "partial\n", encoding="utf-8"
                    )
                    raise autonomous.AutonomousWorkerTimeout(
                        "timed out", "", "partial debug output"
                    )
                return original_run(args, **kwargs)

            with (
                patch.object(autonomous, "WORKTREE_PARENT", worktrees),
                patch.object(
                    autonomous, "_find_opencode", return_value="/fake/opencode"
                ),
                patch.object(autonomous, "_run", side_effect=fake_run),
                patch.dict(os.environ, {"OPENROUTER_API_KEY": "test-key"}),
            ):
                output = autonomous.run_task(
                    repo,
                    {
                        "task": "Edit sample.",
                        "files": ["sample.txt"],
                        "timeout_seconds": 60,
                    },
                )

            self.assertIn("Timed out: yes", output)
            self.assertIn("+partial", output)
            worktree_line = next(
                line for line in output.splitlines()
                if line.startswith("Isolated worktree: ")
            )
            retained = Path(worktree_line.split(": ", 1)[1])
            self.assertTrue(retained.exists())
            subprocess.run(
                ["git", "worktree", "remove", "--force", str(retained)],
                cwd=repo,
                check=True,
            )


if __name__ == "__main__":
    unittest.main()
