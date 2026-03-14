#!/usr/bin/env python3
"""
Minimal MCP server exposing build tools for Hypersomnia.
Tools: build, build_cmake, build_file, cmake_debug_fast
"""

import json
import os
import subprocess
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR = os.path.join(REPO_ROOT, "build", "current")

TOOLS = [
	{
		"name": "build",
		"description": (
			"Build the whole project with ninja. "
			"Returns 'Build succeeded.' on success, or compiler errors on failure. "
			"Runs cmake_debug_fast first if build/ folder doesn't exist yet."
		),
		"inputSchema": {
			"type": "object",
			"properties": {},
			"required": []
		}
	},
	{
		"name": "build_cmake",
		"description": (
			"Regenerate build files by running cmake. "
			"Use when CMakeLists.txt or other cmake files have changed. "
			"Runs cmake_debug_fast first if build/ folder doesn't exist yet."
		),
		"inputSchema": {
			"type": "object",
			"properties": {},
			"required": []
		}
	},
	{
		"name": "build_file",
		"description": (
			"Build a single source file. "
			"Pass the path relative to the repo root, e.g. src/augs/misc/date_time.cpp. "
			"Runs cmake_debug_fast first if build/ folder doesn't exist yet."
		),
		"inputSchema": {
			"type": "object",
			"properties": {
				"path": {
					"type": "string",
					"description": "Source file path relative to repo root, e.g. src/augs/misc/date_time.cpp"
				}
			},
			"required": ["path"]
		}
	},
	{
		"name": "cmake_debug_fast",
		"description": (
			"Generate the 'fast' Debug build configuration (no debug info) using "
			"BUILD_FOLDER_SUFFIX=fast cmake/build.sh Debug x64 -DGENERATE_DEBUG_INFORMATION=0. "
			"Creates build/Debug-x64-clang-fast/ and symlinks build/current to it. "
			"Run this once on a clean repo before using build or build_file."
		),
		"inputSchema": {
			"type": "object",
			"properties": {},
			"required": []
		}
	}
]


def _format_failure(label, returncode, stdout, stderr):
	"""Return a compact failure string, preferring stderr over stdout."""
	output = stderr.strip() or stdout.strip()
	return f"{label} FAILED (exit {returncode}):\n{output}"


def run_cmake_debug_fast():
	env = os.environ.copy()
	env["BUILD_FOLDER_SUFFIX"] = "fast"

	result = subprocess.run(
		["bash", "cmake/build.sh", "Debug", "x64", "-DGENERATE_DEBUG_INFORMATION=0"],
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		text=True,
		cwd=REPO_ROOT,
		env=env
	)

	if result.returncode == 0:
		return "cmake_debug_fast succeeded."

	return _format_failure("cmake_debug_fast", result.returncode, result.stdout, result.stderr)


def _ensure_build_dir():
	"""Run cmake_debug_fast if build/ does not exist. Returns an error string on failure, else None."""
	build_root = os.path.join(REPO_ROOT, "build")
	if not os.path.isdir(build_root):
		result = run_cmake_debug_fast()
		if not result.startswith("cmake_debug_fast succeeded"):
			return result
	return None


def run_build():
	init_err = _ensure_build_dir()
	if init_err:
		return init_err

	if not os.path.isdir(BUILD_DIR):
		return f"Build directory not found: {BUILD_DIR}"

	result = subprocess.run(
		["ninja", "-C", BUILD_DIR],
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		text=True
	)

	if result.returncode == 0:
		return "Build succeeded."

	return _format_failure("Build", result.returncode, result.stdout, result.stderr)


def run_build_cmake():
	init_err = _ensure_build_dir()
	if init_err:
		return init_err

	if not os.path.isdir(BUILD_DIR):
		return f"Build directory not found: {BUILD_DIR}"

	result = subprocess.run(
		["cmake", "../.."],
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		text=True,
		cwd=BUILD_DIR
	)

	if result.returncode == 0:
		return "CMake succeeded."

	return _format_failure("CMake", result.returncode, result.stdout, result.stderr)


def run_build_file(path):
	init_err = _ensure_build_dir()
	if init_err:
		return init_err

	if not os.path.isdir(BUILD_DIR):
		return f"Build directory not found: {BUILD_DIR}"

	path = path.lstrip("./")
	target = f"CMakeFiles/Hypersomnia.dir/{path}.o"

	result = subprocess.run(
		["ninja", "-C", BUILD_DIR, target],
		stdout=subprocess.PIPE,
		stderr=subprocess.PIPE,
		text=True
	)

	if result.returncode == 0:
		return f"Built {path} successfully."

	return _format_failure(f"Build of {path}", result.returncode, result.stdout, result.stderr)


# ---------- MCP protocol helpers ----------

def send(obj):
	sys.stdout.write(json.dumps(obj) + "\n")
	sys.stdout.flush()


def send_response(req_id, result):
	send({"jsonrpc": "2.0", "id": req_id, "result": result})


def send_error(req_id, code, message):
	send({"jsonrpc": "2.0", "id": req_id, "error": {"code": code, "message": message}})


def handle_call(req_id, name, args):
	try:
		if name == "build":
			text = run_build()
		elif name == "build_cmake":
			text = run_build_cmake()
		elif name == "build_file":
			path = args.get("path", "").strip()
			if not path:
				send_error(req_id, -32602, "'path' argument is required")
				return
			text = run_build_file(path)
		elif name == "cmake_debug_fast":
			text = run_cmake_debug_fast()
		else:
			send_error(req_id, -32601, f"Unknown tool: {name}")
			return

		send_response(req_id, {"content": [{"type": "text", "text": text}]})

	except Exception as exc:
		send_error(req_id, -32603, str(exc))


def main():
	for raw in sys.stdin:
		raw = raw.strip()
		if not raw:
			continue

		try:
			msg = json.loads(raw)
		except json.JSONDecodeError:
			continue

		method = msg.get("method", "")
		req_id = msg.get("id")
		params = msg.get("params") or {}

		if method == "initialize":
			send_response(req_id, {
				"protocolVersion": "2024-11-05",
				"capabilities": {"tools": {}},
				"serverInfo": {"name": "hypersomnia-build", "version": "1.0.0"}
			})
		elif method == "initialized":
			pass  # notification — no response
		elif method == "ping":
			send_response(req_id, {})
		elif method == "tools/list":
			send_response(req_id, {"tools": TOOLS})
		elif method == "tools/call":
			handle_call(req_id, params.get("name", ""), params.get("arguments") or {})


if __name__ == "__main__":
	main()
