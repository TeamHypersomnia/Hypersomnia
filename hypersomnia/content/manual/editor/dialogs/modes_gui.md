The Modes dialog lets you tweak the rules for any game mode that is meant to be played on this map.
All rulesets are stored together in a separate ``Project.rulesets`` file in your project directory.
This lets you create your own rules for any existing map - without ever modifying the map file itself.

You can create more than one ruleset for a given game mode.

For example, with a Team Deathmatch:

- You might want to have a set of rules specific for serious 5v5 matches:
	- long freeze times, longer round end delays, no weapons initially granted
- and another set of rules for whenever you want to play a quick 1v1 duel:
	- cap player number to 2, almost instant round restart, 
	  minimal freeze time, maybe always give some basic equipment, and so on.

The server's admin can then load a ruleset of his choice on the fly,
as long as he has the ``.rulesets`` file.

During a playtest, a "Current mode state" node appears in the Modes dialog,
which lets you peek into the state of the currently played mode,
	- Although, that is useful only for debugging purposes.
	- This state is held in the ``Project.player`` file.

## Theory: A solver vs a mode
There are two principal objects responsible for the entire process of moving the game forward in time.

- A solver. It constitutes the great majority of what happens during the game.
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
	- introducing the concept of teams, 
	- counting the rounds, money, and a whole lot of other statistics
	- checking win conditions.
		- For example, if the bomb entity stops existing in Bomb Mode,
		  the win goes to the bombing faction - and the defusing factions loses - 
		  since the only way that a bomb stops existing is due to it exploding.
