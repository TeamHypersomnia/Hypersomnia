#!/usr/bin/env python3
"""
	What the website (hypersomnia.io) shows of the released version, so that it needs nothing from GitHub:
	the weapon stats with the images they name, the rank icons and the official arenas with their miniatures.
	Made by the Linux Headless CI build; the release script puts it in /builds/site_data on hypersomnia.io.

	export_site_data.py --weapon-stats <dir> --release-notes <file> --out <dir> [--content <dir>]

	<out>/
		manifest.json            version, commit, when it was made
		weapons/*.json           as exported by --export-weapon-stats
		gfx/...                  the images the weapons name, and necessary/rank_*.png
		official_arenas.json     name, author, descriptions and version of each official arena
		official_arenas/<name>/miniature.png
"""
import argparse
import datetime
import glob
import json
import os
import shutil
import sys

def copy(src, dst):
	os.makedirs(os.path.dirname(dst), exist_ok=True)
	shutil.copyfile(src, dst)

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('--weapon-stats', required=True)
	parser.add_argument('--release-notes', required=True)
	parser.add_argument('--out', required=True)
	parser.add_argument('--content', default='hypersomnia/content')
	args = parser.parse_args()

	gfx = os.path.join(args.content, 'gfx')
	missing = []

	def copy_gfx(name):
		src = os.path.join(gfx, name)

		if os.path.isfile(src):
			copy(src, os.path.join(args.out, 'gfx', name))
		else:
			missing.append(name)

	stats = sorted(glob.glob(os.path.join(args.weapon_stats, '*.json')))

	if not stats:
		sys.exit(f'No weapon stats in {args.weapon_stats}.')

	for path in stats:
		copy(path, os.path.join(args.out, 'weapons', os.path.basename(path)))

		with open(path) as f:
			for weapon in json.load(f):
				if weapon.get('pic_filename'):
					copy_gfx(weapon['pic_filename'])

				if weapon.get('has_magazine') and weapon.get('magazine_id'):
					copy_gfx(weapon['magazine_id'] + '.png')

	for path in glob.glob(os.path.join(gfx, 'necessary', 'rank_*.png')):
		copy_gfx(os.path.relpath(path, gfx))

	arenas = []

	for folder in sorted(glob.glob(os.path.join(args.content, 'arenas', '*', ''))):
		name = os.path.basename(os.path.normpath(folder))
		project = os.path.join(folder, f'{name}.json')
		miniature = os.path.join(folder, 'miniature.png')

		if not os.path.isfile(project):
			continue

		with open(project) as f:
			arena = json.load(f)

		about = arena.get('about', {})

		arenas.append({
			'name': name,
			'author': about.get('author', ''),
			'short_description': about.get('short_description', ''),
			'full_description': about.get('full_description', ''),
			'version_timestamp': arena.get('meta', {}).get('version_timestamp', '')
		})

		if os.path.isfile(miniature):
			copy(miniature, os.path.join(args.out, 'official_arenas', name, 'miniature.png'))

	# Without the manifest the release script uploads nothing - the website keeps the last complete version
	if missing:
		sys.exit(f'Not in {gfx}: ' + ', '.join(missing))

	with open(os.path.join(args.out, 'official_arenas.json'), 'w') as f:
		json.dump(arenas, f, indent=1)

	with open(args.release_notes) as f:
		notes = f.read().splitlines()

	manifest = {
		'version': notes[0] if notes else '',
		'commit': notes[1] if len(notes) > 1 else '',
		'generated_at': datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%d %H:%M:%S UTC')
	}

	# The manifest last: the website loads everything again when it changes
	with open(os.path.join(args.out, 'manifest.json'), 'w') as f:
		json.dump(manifest, f, indent=1)

	print(f'Site data of {manifest["version"]}: {len(stats)} weapon files, {len(arenas)} official arenas.')

main()
