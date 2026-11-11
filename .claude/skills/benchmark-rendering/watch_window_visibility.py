#!/usr/bin/env python3
import json
import os
import subprocess
import sys
import time

GAME_PID = int(sys.argv[1])
LOG_PATH = sys.argv[2]
SAMPLE_INTERVAL_SECS = 0.5
STACKING_LAYOUTS = ('tabbed', 'stacked')
GAME_WINDOW_NAME_PART = 'hypersomnia'


def run_json(*args):
	return json.loads(subprocess.run(args, capture_output=True, text=True, check=True).stdout)


def client_windows():
	client_list = subprocess.run(['xprop', '-root', '_NET_CLIENT_LIST'], capture_output=True, text=True).stdout
	return {int(part.strip(), 16) for part in client_list.split('#')[-1].split(',') if part.strip()}


def window_name(window_id):
	return subprocess.run(['xprop', '-id', str(window_id), 'WM_NAME'], capture_output=True, text=True).stdout


def new_game_window(windows_before_launch):
	for window_id in client_windows() - windows_before_launch:
		if GAME_WINDOW_NAME_PART in window_name(window_id).lower():
			return window_id

	return None


def find_path(node, window_id, path):
	path = path + [node]

	if node.get('window') == window_id:
		return path

	for child in node.get('nodes', []) + node.get('floating_nodes', []):
		found = find_path(child, window_id, path)

		if found:
			return found

	return None


def window_state(window_id):
	path = find_path(run_json('i3-msg', '-t', 'get_tree'), window_id, [])

	if path is None:
		return None

	visible_workspaces = {w['name'] for w in run_json('i3-msg', '-t', 'get_workspaces') if w['visible']}
	workspace = next((n['name'] for n in reversed(path) if n.get('type') == 'workspace'), None)
	behind_another_tab = any(
		parent.get('layout') in STACKING_LAYOUTS and parent.get('focus') and parent['focus'][0] != child['id']
		for parent, child in zip(path, path[1:])
	)

	if workspace == '__i3_scratch':
		hidden_by = 'scratchpad'
	elif workspace not in visible_workspaces:
		hidden_by = 'another workspace shown'
	elif behind_another_tab:
		hidden_by = 'another tab shown'
	else:
		hidden_by = ''

	return {
		'focused': bool(path[-1].get('focused')),
		'visible': hidden_by == '',
		'hidden_by': hidden_by,
		'workspace': workspace
	}


def game_is_running():
	return os.path.exists('/proc/%d' % GAME_PID)


def main():
	started = time.time()
	windows_before_launch = client_windows()
	window_id = None

	while game_is_running() and window_id is None:
		window_id = new_game_window(windows_before_launch)
		time.sleep(SAMPLE_INTERVAL_SECS / 5)

	seconds = {'sampled': 0.0, 'focused': 0.0, 'visible': 0.0}
	last_state = None

	with open(LOG_PATH, 'w') as log:
		while game_is_running() and window_id is not None:
			state = window_state(window_id)

			if state is not None:
				seconds['sampled'] += SAMPLE_INTERVAL_SECS
				seconds['focused'] += SAMPLE_INTERVAL_SECS if state['focused'] else 0.0
				seconds['visible'] += SAMPLE_INTERVAL_SECS if state['visible'] else 0.0

				if state != last_state:
					log.write('WINDOW %.1f s: focused=%d visible=%d %s (workspace %s)\n' % (time.time() - started, state['focused'], state['visible'], state['hidden_by'], state['workspace']))
					log.flush()
					last_state = state

			time.sleep(SAMPLE_INTERVAL_SECS)

		log.write('WINDOW summary: %.1f s sampled, focused %.1f s, visible %.1f s\n' % (seconds['sampled'], seconds['focused'], seconds['visible']))


if __name__ == '__main__':
	main()
