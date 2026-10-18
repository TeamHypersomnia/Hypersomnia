#!/usr/bin/env python3
"""
Copies a map under a new name and edits its JSON - for testing a map with other rules or items.

    map_copy.py <source map dir> <dest parent dir> <new name> [path=json_value ...]

e.g. map_copy.py hypersomnia/content/arenas/de_cyberaqua $SCRATCH de_snipers \
        'game_modes.bomb_defusal.shop_allowed_firearms=["hunter","awka"]' 'game_modes.bomb_defusal.round_time=30'

The new name matters: a map named like one in content/arenas or user/downloads/arenas is shadowed by it,
and the JSON file inside has to be named after the folder. Prints the new map dir.
"""
import json, os, shutil, sys

src, dest_parent, name = sys.argv[1], sys.argv[2], sys.argv[3]
old_name = os.path.basename(os.path.normpath(src))
dest = os.path.join(dest_parent, name)

if os.path.exists(dest):
	sys.exit(f"{dest} exists - pick another name (never deleted here)")

shutil.copytree(src, dest)
os.rename(os.path.join(dest, old_name + ".json"), os.path.join(dest, name + ".json"))

json_path = os.path.join(dest, name + ".json")

with open(json_path) as f:
	data = json.load(f)

for assignment in sys.argv[4:]:
	path, value = assignment.split("=", 1)
	keys = path.split(".")
	node = data

	for k in keys[:-1]:
		node = node.setdefault(k, {})

	node[keys[-1]] = json.loads(value)

with open(json_path, "w") as f:
	json.dump(data, f, indent=4)

print(dest)
