The Modes gui lets you tweak the rules for any game mode that is meant to be played on this map.
All rulesets are stored in a separate "Project.rulesets" file in your project directory.
This lets you create your own rulesets any map without ever modifying the map file itself.

You can create more than a single ruleset 

predefined sets of rules for modes that will be play

During a playtest, it also lets you peek into the state of the currently played mode.
	- Although, that is useful only for debugging purposes.
	- This state is held in the 


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
