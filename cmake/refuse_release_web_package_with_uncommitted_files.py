#!/usr/bin/env python3
import re
import subprocess
import sys

RESOURCES_DIR = sys.argv[1]
GENERATED_VERSION_FILE = sys.argv[2]
PACKAGED_PATHS = ['content', 'detail', 'default_config.json', 'CHANGELOG.md']
DEVELOPMENT_VERSIONS = ['', '0.0.0']


def git_lines(*arguments):
	result = subprocess.run(['git', '-C', RESOURCES_DIR, *arguments], capture_output=True, text=True)

	if result.returncode != 0:
		sys.exit('Cannot tell if the packaged game files match the commit this build names:\n%s' % result.stderr)

	return [line for line in result.stdout.split('\n') if line]


def field_of_generated_version(name):
	with open(GENERATED_VERSION_FILE, encoding='utf-8') as version_file:
		found = re.search(r'%s\("([^"]*)"\)' % name, version_file.read())

	return found.group(1) if found else ''


def main():
	version = field_of_generated_version('commit_tag')

	if version in DEVELOPMENT_VERSIONS:
		return

	commit = field_of_generated_version('commit_hash')
	changed = git_lines('diff', '--name-only', commit, '--', *PACKAGED_PATHS)
	untracked = git_lines('ls-files', '--others', '--exclude-standard', '--', *PACKAGED_PATHS)
	differing = changed + untracked

	if not differing:
		return

	sys.exit(
		'REFUSED: this web build calls itself %s but would pack game files that differ from its commit %s,\n'
		'so official maps would not match the servers ("The local arena file differs from the servers").\n'
		'Commit, stash or revert these files first, or regenerate the version file:\n%s'
		% (version, commit[:9], '\n'.join(differing))
	)


if __name__ == '__main__':
	main()
