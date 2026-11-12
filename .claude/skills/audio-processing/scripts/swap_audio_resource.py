#!/usr/bin/env python3
import json
import os
import shutil
import subprocess
import sys

SCRIPTS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPTS_DIR, '..', '..', '..', '..'))
BLAKE3_DIR = os.path.join(REPO_ROOT, 'src', '3rdparty', 'blake3')
BLAKE3_STDIN_HASHER = os.path.join(os.environ.get('XDG_CACHE_HOME', os.path.expanduser('~/.cache')), 'hypersomnia-blake3-stdin')


def built_blake3_stdin_hasher():
	if not os.path.exists(BLAKE3_STDIN_HASHER):
		sources = [os.path.join(BLAKE3_DIR, name) for name in ['example.c', 'blake3.c', 'blake3_dispatch.c', 'blake3_portable.c']]
		portable_only = ['-DBLAKE3_NO_SSE2', '-DBLAKE3_NO_SSE41', '-DBLAKE3_NO_AVX2', '-DBLAKE3_NO_AVX512']
		os.makedirs(os.path.dirname(BLAKE3_STDIN_HASHER), exist_ok=True)
		subprocess.run(['cc', '-O2', '-I' + BLAKE3_DIR] + portable_only + sources + ['-o', BLAKE3_STDIN_HASHER], check=True)

	return BLAKE3_STDIN_HASHER


def blake3_of(path):
	with open(path, 'rb') as contents:
		return subprocess.check_output([built_blake3_stdin_hasher()], stdin=contents).decode().strip()


def game_is_running():
	return subprocess.run(['pgrep', '-x', 'Hypersomnia'], capture_output=True).returncode == 0


def main():
	project_json, resource_id, new_audio = sys.argv[1], sys.argv[2], sys.argv[3]

	if game_is_running():
		sys.exit('Hypersomnia is running - close it first, the editor would overwrite the project')

	with open(project_json) as project_file:
		text = project_file.read()

	resource = next((r for r in json.loads(text).get('external_resources', []) if r.get('id') == resource_id), None)

	if resource is None:
		sys.exit('%s is not a resource of %s' % (resource_id, project_json))

	old_hash_field = '"file_hash": %s' % json.dumps(resource.get('file_hash', ''))

	if not resource.get('file_hash') or text.count(old_hash_field) != 1:
		sys.exit('%s has no file_hash of its own to replace - save the project in the editor first' % resource_id)

	destination = os.path.join(os.path.dirname(os.path.abspath(project_json)), resource['path'])

	new_hash = blake3_of(new_audio)
	updated = text.replace(old_hash_field, '"file_hash": %s' % json.dumps(new_hash))
	json.loads(updated)

	if not (os.path.exists(destination) and os.path.samefile(new_audio, destination)):
		shutil.copyfile(new_audio, destination)

	with open(project_json, 'w') as project_file:
		project_file.write(updated)
	print('%s -> %s, file_hash %s' % (resource_id, resource['path'], new_hash))


if __name__ == '__main__':
	main()
