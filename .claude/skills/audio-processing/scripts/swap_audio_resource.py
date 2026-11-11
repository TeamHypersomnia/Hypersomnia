#!/usr/bin/env python3
import json
import os
import re
import shutil
import subprocess
import sys

SCRIPTS_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPTS_DIR, '..', '..', '..', '..'))
BLAKE3_DIR = os.path.join(REPO_ROOT, 'src', '3rdparty', 'blake3')
B3SUM = os.path.join(os.environ.get('XDG_CACHE_HOME', os.path.expanduser('~/.cache')), 'hypersomnia-b3sum')


def built_b3sum():
	if not os.path.exists(B3SUM):
		sources = [os.path.join(SCRIPTS_DIR, 'b3sum.c')] + [os.path.join(BLAKE3_DIR, name) for name in ['blake3.c', 'blake3_dispatch.c', 'blake3_portable.c']]
		portable_only = ['-DBLAKE3_NO_SSE2', '-DBLAKE3_NO_SSE41', '-DBLAKE3_NO_AVX2', '-DBLAKE3_NO_AVX512']
		os.makedirs(os.path.dirname(B3SUM), exist_ok=True)
		subprocess.run(['cc', '-O2', '-I' + BLAKE3_DIR] + portable_only + sources + ['-o', B3SUM], check=True)

	return B3SUM


def blake3_of(path):
	return subprocess.check_output([built_b3sum(), path]).decode().split()[0]


def resource_block_span(text, resource_id):
	anchor = text.index('"id": %s' % json.dumps(resource_id))
	start = text.rindex('{', 0, anchor)
	depth = 0

	for i in range(start, len(text)):
		if text[i] == '{':
			depth += 1
		elif text[i] == '}':
			depth -= 1

			if depth == 0:
				return start, i + 1

	raise ValueError(resource_id)


def game_is_running():
	return subprocess.run(['pgrep', '-x', 'Hypersomnia'], capture_output=True).returncode == 0


def main():
	project_json, resource_id, new_audio = sys.argv[1], sys.argv[2], sys.argv[3]

	if game_is_running():
		sys.exit('Hypersomnia is running - close it first, the editor would overwrite the project')

	text = open(project_json).read()
	start, end = resource_block_span(text, resource_id)
	block = text[start:end]
	resource_path = json.loads(block)['path']
	destination = os.path.join(os.path.dirname(os.path.abspath(project_json)), resource_path)

	if not os.path.samefile(new_audio, destination):
		shutil.copyfile(new_audio, destination)

	new_hash = blake3_of(destination)
	block = re.sub(r'"file_hash": "[0-9a-f]+"', lambda _: '"file_hash": "%s"' % new_hash, block, count=1)
	updated = text[:start] + block + text[end:]
	json.loads(updated)
	open(project_json, 'w').write(updated)
	print('%s -> %s, file_hash %s' % (resource_id, resource_path, new_hash))


if __name__ == '__main__':
	main()
