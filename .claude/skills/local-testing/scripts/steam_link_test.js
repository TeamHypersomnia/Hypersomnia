// Linking by a Steam ticket (link_key.js linkBySteamTicket) at the 32-key limit, straight on a scratch site DB.
// Copy into the scratch site dir (broker.sh) and run there: node steam_link_test.js
const db = require('./src/db');
const lk = require('./src/link_key');
const crypto = require('crypto');
const key = () => 'key_' + crypto.randomBytes(32).toString('hex');
const sid = () => 'steam_7656' + crypto.randomInt(1e9, 9e9) + '000';
let failed = 0;
const check = (ok, what) => { console.log((ok ? 'PASS ' : 'FAIL ') + what); if (!ok) failed++; };
const parent = k => (db.prepare('SELECT parent_id FROM associations WHERE child_id = ?').get(k) || {}).parent_id;
const via = k => (db.prepare('SELECT linked_via FROM key_links WHERE child_id = ?').get(k) || {}).linked_via;
const browserLink = (k, s) => { db.prepare('INSERT INTO associations (child_id, parent_id) VALUES (?, ?)').run(k, s);
  db.prepare("INSERT OR REPLACE INTO key_links (child_id, linked_via, device, last_cert_at) VALUES (?, 'browser', NULL, ?)").run(k, 1); };

// 4: B full of browser keys - K stays on A
const A = sid(), B = sid(), K = key();
browserLink(K, A);
for (let i = 0; i < 32; i++) browserLink(key(), B);
lk.linkBySteamTicket(K, B, 'dev');
check(parent(K) === A, `full account of browser keys: the key stays on its old account (${parent(K)})`);

// 5: eviction keeps the Steam account's progress
const C = sid(), E = key(), K2 = key();
for (let i = 0; i < 31; i++) browserLink(key(), C);
db.prepare('INSERT INTO associations (child_id, parent_id) VALUES (?, ?)').run(E, C);
db.prepare("INSERT OR REPLACE INTO key_links (child_id, linked_via, device, last_cert_at) VALUES (?, 'steam', 'old', 1)").run(E);
db.prepare('INSERT OR REPLACE INTO mmr_team (account_id, mmr, mu, sigma, matches_won, matches_lost) VALUES (?, 30, 30, 5, 3, 1)').run(C);
db.prepare('INSERT OR REPLACE INTO progress_moves (child_id, parent_id, table_name, matches_after_move) VALUES (?, ?, ?, ?)').run(E, C, 'mmr_team', 4);
lk.linkBySteamTicket(K2, C, 'dev');
check(parent(E) === undefined, 'the oldest Steam build key was let go');
check(parent(K2) === C, 'the new key is linked');
check(!!db.prepare('SELECT 1 FROM mmr_team WHERE account_id = ?').get(C), 'the Steam account kept its progress');
check(!db.prepare('SELECT 1 FROM mmr_team WHERE account_id = ?').get(E), 'the let go key got no progress back');

// 5b: relink of a browser key by ticket keeps linked_via
const D = sid(), K3 = key();
browserLink(K3, D);
lk.linkBySteamTicket(K3, D, 'dev2');
check(via(K3) === 'browser', `linked_via stays browser (${via(K3)})`);
console.log(failed + ' failed');
process.exit(failed ? 1 : 0);
