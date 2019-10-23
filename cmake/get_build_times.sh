#!/usr/bin/env bash 
python cmake/diff_columns.py build/current/.ninja_log | sort -nr > /tmp/build_times.txt
$VISUAL /tmp/build_times.txt
