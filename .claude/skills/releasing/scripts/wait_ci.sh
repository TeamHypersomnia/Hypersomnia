#!/bin/bash
# wait_ci.sh <sha> - prints the 4 CI builds of that commit every minute until all completed
cd "$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
SHA=$1
while true; do
	out=$(gh run list --branch master --limit 12 --json headSha,status,conclusion,workflowName 2>/dev/null | python3 -c "
import json,sys
r=[x for x in json.load(sys.stdin) if x['headSha'].startswith('$SHA') and x['workflowName']!='pages-build-deployment']
print(len(r), sum(x['status']=='completed' for x in r), ' '.join(x['workflowName'].split()[0]+':'+(x['conclusion'] or x['status']) for x in r))")
	echo "$(date +%H:%M) $out"
	set -- $out
	[ "$1" = 4 ] && [ "$2" = 4 ] && break
	sleep 60
done
