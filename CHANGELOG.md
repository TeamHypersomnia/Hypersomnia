# Changelog

1. MAJOR version marks a significant milestone (e.g. bots, scripting).
2. MINOR version marks a network protocol-breaking change.
3. PATCH version marks a backward compatible feature or fix.

## [unreleased] - 2025-07-06

### Added

- Clients can set their *Clan* in addition to their Nickname (`Settings -> Client`).
	- The *Clan* will appear in the scoreboard (Tab).
- `server_private.custom_webhook_urls` - Matrix-friendly webhooks.
	- Can have multiple targets that filter by *Clan*.
	- A single server keep multiple communities up-to-date with events only relevant to them.
	- E.g. if a player from a given _Clan_ connects to the server, only their _Clan_ will be notified.
	- This also relays chat messages now.
- Basic bot spawning/kicking logistics.
	- `/bots m [y]` command. `/bots 4` sets total quota, `/bots 1 4` specifies a custom split.
	- Taking control of the bot upon player death.
	- Reporting number of online bots/humans in the server list.
	- Bots are still stationary.

### Changed

- Can always use `/map` if all players are on the same team (so e.g. **if alone** on the server).
- Improved config scheme:
	- Renamed `config.json` to `runtime_prefs.json` to avoid ambiguity.
	- `runtime_prefs.json` now **takes precedence** over `conf.d/`.
		- This means that RCON changes **are now persistent** between server restarts.
	- RCON window now shows the content of `runtime_prefs.json` and you can reset all the RCON-made changes with a button click.

### Fixed

- **Balance:** `ao44` and `covert` prices have been swapped.
- Finally fix the goddamn clipboard in Linux Steam builds.
- FFA/gun game match results now report correctly through the webhooks.
- `user/conf.d` will only consider `.json` files from now to prevent crash when a non-json file is accidentally put there.
- `[Web]` was incorrectly added to the native client's nickname when connecting to a server hosted in the browser.

### Notes

- This is the first tagged version. Previous work existed but wasn't versioned.
