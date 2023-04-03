---
title: ToDo bugs
hide_sidebar: true
permalink: todo_bugs
summary: Just a hidden scratchpad.
---

- linux bug: neon silhouettes can be seen behind player, probably something to do with drivers
	- It's a problem with gl_FragCoord: probably stencil on another fbo is somehow flipped
	- To reverse the problem, one can put the following in fog_of_war.fsh: layout(origin_upper_left) in vec4 gl_FragCoord; 
		- although only with higher glsl version
	- didn't happen before, very probable it's a driver bug

- Fixing wallbangs and wall teleportation
    - A short raycast sensor in front of the player
        - Detect all convex shapes of walls in front of him
        - Just take all vertices of all detected convexes
        - And create a single trapezoid body
    - Don't do raycast
        - simply query the  right before the physics step (after applying the crosshair's direction to player rotation)

- There was a crash due to dead entity in driver setup finding a component
	- possible cause is that the gun system destroys item in chamber, but technically they should not trigger END_CONTACT
	as they are devoid of... hey, actually that might be because we still see those entities even if they are in backpack. 
	In that case, if the owner body of an item chamber is badly determined, it might make sense that it crashes
	- ensure that the posted subject in contact listener is alive, but that is most likely the case since we get physical components from them anyway
		- but ensures won't hurt

# Disregarded

- Investigate the mysterious hotbar crash
	- Happened when we finished drag and dropping on hotbar 
	- All buttons on hotbar were assigned
	- The item we dragged was hidden in backpack and *possibly* unassigned to hotbar yet
	- NEW: The crash happened EXACTLY when pressing an AWP that was in the backpack
		- But it was presuambly not in the hotbar since we pressed it
	- haven't seen for a while and inventory will be reworked anyway

- NAT traversal does not always work with our symmetric port-sensitive
	- Working connections:
		- filemon -> Billan
		- Pythagoras <-> Billan
		- Pythagoras <-> Shuncio
		- Pythagoras -> cold dimensions
		- Pythagoras -> Wojteg
	- Not working:
		- filemon <-> Pythagoras
	- not so important, and it's from a long time ago, generally it works if the nat does not randomize ports

- GCC relwithdebinfo does not properly set the physical shapes
	 - ?


- Possibly disallow some editor ops when in gameplay/gui mode?
	- E.g. there was a crash after instantiating cause the world cursor pos could not be found in gameplay mode
	- new editor will solve this

- fix freetype charsets
	- ?

# Done

- remember to fix your system time before hosting
	- fix that damn connect token

- wrong interpolation values in client setup
	- But somehow only for fy_minilab
		- It was probably generated with 128 so the solvable might still have 128 written into it
		- but only predicted cosmos sometimes shows 128, referential always has 60
			- how is it possible if the only source of state for predicted is the referential?
				- maybe referential is somehow updated later? And predicted is only rewritten on mismatch afaik
				- yeah first both are read from disk, only then is initial state applied
					- after reading from disk both should have 128
					- then upon receiving initial, is predicted properly set to 60?
- warmup not played when switching maps sometimes and still spectating (in release)
	- solved, due to the above

- Wandering pixels crash
	- Fixed by eliminating Data race, see the comments in audiovisual_state.cpp

- Server crash from 11.01.2023 at 01:24 - fixed with *11b30d931246caa7e0cce04d7e83756eb33acd53*
	- Kartezjan then najsilniejszy programista C++
	- It crashed before successfully sending the message to Telegram but after successfully sending the Discord one
	- Actually rather peculiar, if both lived enough for the discord message to be properly sent, there's no reason telegram one wouldn't eventually arrive, must've been problem elsewhere 
		- I've also seen kartezjan for a bit so it must've sent me some data
	- Maybe httplib is not actually thread safe
		- Just in case I'd update it and merge the two webhook handlers into a single lambda

- Enabling HRTF in debug mode crashes the game but there's no core or segfault or anything
	- it just quits
	- Fixed by updating OpenAL
