The Modes dialog lets you tweak the rules for any game mode that is meant to be played on this map.
All rulesets are stored together in a separate ``Project.rulesets`` file in your project directory.
This lets you create your own rules for any existing map - without ever modifying the map file itself.

You can create more than one ruleset for a given game mode.

For example, with a Team Deathmatch:

- You might want to have a set of rules specific for serious 5v5 matches:
	- long freeze times, longer round end delays, no weapons initially granted
- You might also want to have an additional set of rules for occasions when you want to play a quick 1v1 duel:
	- cap player number to 2, almost instant round restart, 
	  minimal freeze time, maybe always give some basic equipment, and so on.

The server admin can then load a ruleset of his choice on the fly,
as long as they have the ``.rulesets`` file.

During a playtest, a "Current mode state" node appears in the Modes dialog,
which lets you peek into the state of the currently played mode,
	- Although, that is useful only for debugging purposes.
	- This state is held in the ``Project.player`` file.

## Default rulesets

If a ruleset is specified as a "Playtest default",
it will be chosen for playtesting in editor whenever you press ``i`` 
to enter gameplay for the first time.
A yellow "P" will appear next to its name.
Of course, there can only be a single Playtest default at a time.

If a ruleset is specified as a "Server default",
it will be loaded whenever this map is chosen for playing on any game server.
	Of course, the server admin will later be able to choose among all modes available.
An orange "S" will appear next to its name.
Of course, there can only be a single Server default at a time.

## What exactly is a mode? A mode versus a solver.

There are two principal objects responsible for the entire process of moving the game forward in time.

- A solver. It constitutes the majority of what happens during the game.
  During a solve, these things take place, among many others:
	- physics and collision handling,
	- detonation of bombs and grenades,
	- initiation of shots from pressed guns,
	- deaths of sentient beings with nonpositive health,
	- performing of spell logic.
  A solver is like a mindless, cold universe that relentlessly carries on the stones once set in motion.
- A mode. It introduces a meaning to the game by implementing some actual goals.
  An example of a mode is a Team Deathmatch, a Bomb Mode, or a Free for All.
  A mode is responsible for:
	- respawning dead sentient beings back to life when it is appropriate,
		- e.g. when a new round starts or right away during a warmup.
	- introducing the concept of teams if so is appropriate, 
	- keeping track of rounds, scores, money, and a whole lot of other statistics,
	- checking win conditions.
		- For example, if the bomb entity stops existing in Bomb Mode,
		  the win goes to the bombing faction - and the defusing faction loses - 
		  since the only way that a bomb stops existing is due to it exploding.
