---
title: ToDo future
hide_sidebar: true
permalink: todo_bugs
summary: Just a hidden scratchpad.
---

- MacOS fix: make non-raw input work
	- although this will only bite you in the ass with reveal to explorer etc, and even then not necessarily

- lifting system
	- small/light objects become part of the player fixtures exactly as they were picked, LPM then throws and RPM puts down
	- large objects can only be pulled
		- some simple joint could be at work or just bidirectional seeking behavior

- Fun idea for crouch-behind implementation
	- A shoot-through surface becomes non-see-through when we're close to it while crouching
		- It becomes see through immediately when we aim towards it (simple raycast test)
		- Then aiming towards immediately counts as "standing up"
	- This would require some 

- Modes should be able to specify enabled/disabled layers
	- This way we can encode an alternative destroyed environkment for a zombie mode, in the same map! Instead of creating e.g. zm_cyberaqua
		- Ofc this would also need an ambient light color override

- Easiest destructon system ever
	- Just have a list of remnant flavors
	- Along with positions and rotations
	- Destruction system is responsible literally for just spawning them when enough damage is applied
		- This will avoid the necessity to calculate some polygonal remnants
		- Remnants could be destructible as well and have their own user-defined physical shapes

- Smoke grenades
	- Obstructing visibility
		- Actually very simple: after visibility polygon is already generated:
			- for every smoke circle, render a simple black polygon 
			- the polygon is similar to trapezoid: shorter side begins in the circle, larger side ends off-screen
		- No raycasts needed at all - this will be render-bound which is good
	- We could easily make ones that expand to fill the space like in CS 2
		- We can incrementally spawn squares that expand and scan around
			- These could even be sensors
		- Actually a BFS
			- terminates recursion after the first static obstacle is detected, but still considers the colliding square node
	- Interaction will be harder
		- Actually very simple
		- The squares don't have to interact with anything but bullets and explosions
		- They don't have to move at all
		- Whenever they are out of their current position, however, they seek back

- Letting servers adjust the speed of the game
	- bomb mode doesn't do timing, it just advances whenever asked, but it has to effecctively use the delta information
		- which is obtained by ls / tickrate
	- Remember to never let the incremented timers be treated as the system time
		- Not so important for view of the arena modes as they are several mintues at most
	- The tickrate and the logic speed multiplier (LSM) is transparent to the cosmos, it just gets a different delta
	- dt_secs = LSM / tickrate;
		- dt_secs here is not equal to the real passed time
	- due to limitations, can only be set for when a new round starts
		- could be in rules, and just applied whenever an initial cosmos is assigned from
	- updaterate should as well be different, e.g. with 144 hz we might want to send packets at rate of 72 times per second
	- If we are getting time values for an arena mode, they have to be multiplied by logic speed
	- we should let a map select some sensible defaults?
	- audiovisual state can accept speed mult separately
		- which could be also changed when a proper tick is check

- maps as git repositories
	- how do we facilitate modifications on existing maps so that they don't have to be re-downloaded?
	- we'd have to add remotes and assign branches to them

- fix bots 
	- They currently allocate ids supposed to be used by clients and it all gets screwed

- We could log each respective step of the fp consistency test (could be enabled via config) to diagnose where we diverge
	- Looks like Windows with STREFLOP yields even a different result than both windows without streflop and linux with streflop

- Tinfoil hat
	- Grants a 50% protection from magic
	- Can't use spells
	- 1900$

- Switching to fixed point
	- Fix cases like rng.randval(-1.f, 1.f) where the template will resolve to floats

- Maps could be git repositories, maybe even hosted on github
	- Server could specify upstream url
	- Updating a map just involves pulling

- window should not be concerned with mouse pos pausing and last mouse pos.
	- why lol?

- Instead of having "force joint" at all, make it so that while processing the cars, they additionally apply forces to drivers to keep them

- cosmos::retick 
	- that can change delta while preserving timings by updating all stepped timestamps according to lifetimes found in other places

- Cars
	- Let car calculate its flags statelessly from movement flags in the movement component of the driver?
		- less noise in the cosmos indeed
	- cars could just hold several particle effect inputs and we would iterate cars to also perform
		- handle cars later please
		- particles simulation system can have a "iterate cars and perform engine particles"
			- would just create particle effect inputs and pass them to the simulation
		- same with sound system

