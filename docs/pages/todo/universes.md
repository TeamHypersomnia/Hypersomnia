# Universes - one session per account across servers (not built yet)

- Goal: an account holds a progress session (ranked, later exp/PvP) in one place at a time - no strong character on two servers at once.
- Universe: servers whose progress is one. Official servers are one universe (linked by their keys in official_servers.d).
  A community universe: its servers share `.keys/universe.key`; without it a server is a universe of its own (`server.key`).
- A small isolated HTTPS service (on the masterserver by default) - no database: sessions in memory (they live ~30 s and are
  refreshed anyway), signatures checked with universe keys, the official servers' keys from a file.
- A server may configure another session endpoint (its own identity infrastructure), like the broker keys in its config.
- Messages from a server, signed with its universe key and a timestamp (no replays):
  - at join: "account X (device key D) joins me in universe U" -> admit / refuse;
  - every ~15 s: the list of its progress sessions (a missing one expires after ~30 s, e.g. a crash);
  - "X left": frees the account at once.
- Between different devices the first session wins: the second server refuses ("Your account is already playing on <server>
  from another device (<device>)"); the old session is never kicked.
- The same device takes its session over at once (fast switching, a crash, a ranked rejoin); the old server learns it in the
  answer to its next refresh and kicks that client if still there (two games on one computer).
- Sessions per (universe, account) - universes can not touch each other; a cap on records per universe.
- Service down or just started (one refresh period): no new ranked sessions; players already in keep playing; exp servers play
  without earning progress until confirmed.
- No check in report_match - official servers are trusted with the progress they report.
