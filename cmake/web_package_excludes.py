#!/usr/bin/env python3
import os
import shlex
import subprocess
import sys

RESOURCES_DIR = sys.argv[1]
RESPONSE_FILE = sys.argv[2]
PACKAGED_FOLDERS = ['content', 'detail']
FNMATCH_SPECIAL_CHARACTERS = '[]*?'


def git_ignored_paths():
	try:
		listing = subprocess.run(
			['git', '-C', RESOURCES_DIR, 'ls-files', '-z', '--others', '--ignored', '--exclude-standard', '--directory', '--', *PACKAGED_FOLDERS],
			capture_output=True,
			check=True
		).stdout
	except (OSError, subprocess.CalledProcessError):
		return []

	return [os.path.join(RESOURCES_DIR, path.rstrip('/')) for path in listing.decode('utf-8').split('\0') if path]


def as_literal_fnmatch_pattern(path):
	return ''.join('[%s]' % character if character in FNMATCH_SPECIAL_CHARACTERS else character for character in path)


def main():
	lines = ['--exclude-file %s\n' % shlex.quote(as_literal_fnmatch_pattern(path)) for path in sorted(git_ignored_paths())]
	new_content = ''.join(lines)

	if os.path.exists(RESPONSE_FILE):
		with open(RESPONSE_FILE, encoding='utf-8') as existing:
			if existing.read() == new_content:
				return

	with open(RESPONSE_FILE, 'w', encoding='utf-8') as response_file:
		response_file.write(new_content)


if __name__ == '__main__':
	main()
