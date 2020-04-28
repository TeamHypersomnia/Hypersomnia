# Modes, rules and schemes

- "rules.yml" format everywhere
	- For simplicity: Monolithic. Single file specifies vars for all possible game modes.
		- So hold a tuple of all mode-specific RULES (not rulesets) structs
	- The C++ game mode code later only takes relevant structs as arguments, anyway.

- In the order of application:
	- official/common.rules.yml
	- (Votable) Difficulty
		- casual.rules.yml, OR
		- competitive.rules.yml
	- project/rules/common.rules.yml
	- (Optional, Votable) Custom scheme (project/rules/schemes)
	- "Session"/"Ad hoc" rules specified through RCON; these are always overlaid on top
		- A separate place in RCON to list all overrides

These apply to all game modes (FFA, TDM, CTF).
Game modes are votable too.

A map specifies the list of game modes that it supports,
and it is expected that every scheme will fit all supported game modes.

E.g. aim_pistols and aim_rifles will fit TDM, CTF, Defusal just fine.

In particular, neither Rules nor Schemes will not specify a game mode.

Current scheme, diffiuclty are simply server vars (or solvable server vars).

- Examples
	- We want to have an aim map.
		- Specify supported: CTF, Deathmatch, Defusal (so that the other can't camp)
		- Custom schemes: aimpistol, aimsniper
