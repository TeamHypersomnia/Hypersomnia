#!/bin/bash
set -e
recording=${RECORDING:?RECORDING=<project>/.recorded_playtests/<date> is required}
backups=${BACKUPS:?BACKUPS=<a persistent folder for recording backups, never the scratchpad> is required}
recording=$(cd "$recording" && pwd)
project=$(dirname "$(dirname "$recording")")
map=${MAP:-$project/$(basename "$project").json}
stamp=$(date +%F_%H-%M-%S)
backup=$backups/$(basename "$project")/$(basename "$recording")__before_map_injection_$stamp

[ -f "$map" ] || { echo "no map json at $map - pass MAP="; exit 1; }

mkdir -p "$(dirname "$backup")"
cp -r "$recording" "$backup"
echo "backed up the recording to $backup"

cp "$map" "$recording/map.json"
echo "injected $map into $recording/map.json - capture it now (capture_replay.sh)"
