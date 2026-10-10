#!/bin/bash
# Shows (or changes) the test site's database on arenaus. Usage: db.sh show | db.sh sql "<statement>"
if [ "$1" == "sql" ]; then
	timeout 60 ssh arenaus "python3 -c \"
import sqlite3,sys
c=sqlite3.connect('/home/ubuntu/hs-test-backend/page/private/mmr.db')
with c: n=c.execute(sys.argv[1]).rowcount
print('rows affected:', n)\" \"$2\""
	exit
fi

timeout 60 ssh arenaus "python3 - <<'EOF'
import sqlite3, json
c = sqlite3.connect('file:/home/ubuntu/hs-test-backend/page/private/mmr.db?mode=ro', uri=True)
print('mmr_team:')
for r in c.execute('select account_id, nickname, round(mmr,2), matches_won, matches_lost from mmr_team order by account_id'):
    print('  ', r[0][:24], r[1], r[2], 'W', r[3], 'L', r[4])
print('associations:')
for r in c.execute('select child_id, parent_id from associations'):
    print('  ', r[0][:24], '->', r[1])
print('matches:')
for r in c.execute('select match_id, server_id, server_name, win_score, lose_score, winners, losers from matches order by match_id'):
    w = [x['id'][:16] for x in json.loads(r[5])]
    l = [x['id'][:16] for x in json.loads(r[6])]
    print('  ', r[0], r[1], r[2], str(r[3]) + ':' + str(r[4]), 'W', w, 'L', l)
EOF"
