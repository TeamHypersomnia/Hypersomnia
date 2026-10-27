#!/bin/bash
# hostcheck.sh - run on a host (ssh host bash -s < hostcheck.sh): its game file, version, service state, uptime
F=$(systemctl cat hypersomnia 2>/dev/null | grep ExecStart | awk '{print $1}' | cut -d= -f2)
echo "plik: $F"
ls -la "$F" 2>&1 | cut -c1-90
"$F" --version 2>/dev/null | head -1
systemctl is-active hypersomnia
ps -eo etimes,args | grep "[u]sr/bin/Hypersomnia" | awk '{print "uptime " $1 "s"}' | head -1
