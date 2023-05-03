---
title: ToDo bugs
hide_sidebar: true
permalink: todo_bugs
summary: Just a hidden scratchpad.
---


- caki crash
	- changing fy minilab reloaded remake to de_labs2
		- vars was null
	- message.txt 
- lopa crash
	- when pressing OK 
	- we have logs on discord


- Weapon collider is fucked when it's in the other hand


We have a demo file


- don't autolaunch on the replayed demo!!!!! because it might crash

- on de_duel_practice, we had an openal error because of a reseek_to_sync_if_needed on a warmup theme
	- alSourcef was called with an out of range value, but I'm not sure how it's possible since we clamp the value always to the duration
	- However simply setting stop (stop is always valid for a source) fixes the issue for now, but I'm not sure how it happened in the first place

- crash on bigilab: we should be able to reproduce it with a demo on windows

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

- all_necessary_sounds might cause a crash on destruct too

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

- Server sent entropy too early?
    - server wasn't pausing the solvable stream for downloading clients when sending entropies
- TLSF crashes after sending 9 files
    - Maybe we're not freeing the blocks properly
    - Probably an issue with tlsf, wouldn't be surprised if it was because of differing sizes for files, at least one block was remaining per each size
        - which is why we need to use default allocator for our block messages because their sizes will be nondeterministic
            - this will also free our per-client memory

- demo files might be fucked up during sessions with downloaded maps
	- they must correctly remember the full state snapshots
	- It was just a matter of ignoring file payloads

- When comparing dumped solvables - aligned storage DOES NOT CALL CONSTRUCTORS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    - this is why you get differing results when comparing binary .solv files
    - we might zero it out for investigations
    - we could also use const size vector to zero out once we clear fixtures etc to look for the crash
	- although std::vector::clear would normally trigger a sigsegv too


- Desync with hajt
	- Try playing it on windows
	- Happens when damage is dealt for some reason
		- couldnt physics break at these bullet speeds?
	- Perhaps rapidjson is a source of indeterminism when loading doubles/floats
- The desync happened with claarity too on a very simple map so it makes me think it's an issue with how it's loaded from json

- Crash due to yojimbo_assert( sentPacket );
	- Only in debug build though
	- This doesn't really break anything if we just comment it out and let it go
		- this is basically because we don't send any packets on all the other channels for a while so they have m_sequence = 0, and they are suddenly greeted with a very large sequence number
		- the question being, if it's not inserted into the sequence buffer, how the hell is it even getting acked
			- so why does this just work?
		LOG_NVPS(sequence,m_sentPackets->GetSequence(), m_sentPackets->GetSize());
	- looks like the sequence number is increased by just acking the fragments, it won't help to withhold file requests
- Found culprit:
	auto step_input = logic_step_input { scene.world, entropy, solve_settings() };
	- this caused fish to not simulate during first step which later led to a desync
- Enough to change to:
	auto settings = solve_settings();
	auto step_input = logic_step_input { scene.world, entropy, settings };
- silly C++ just let us literally compile UB..
	- this would normally be catched by static analysis so we should do this

- Might also later fix the direct/indirect memory leaks reported by address sanitizer
	- done for box2d


## SOLVED - crash when cloning entities

Happened:
- when quick testing arena5, probably due to item deletion?
- On claarity's server, again the same similar bug

Solved:

- SIGSEGV was due to pool reallocating and invalidating all entity handles and component references.

- Note the pool didn't actually call reserve but it was objects.push_back that did it
	- Most probably as the pool was read from the network it didn't reserve a capacity for objects vector
	- But it read indirectors vector and indirectors.size() is taken as pool's capacity() so it didn't need to explicitly reserve

## Rest

- why prod-debug doesn't see details.lua? wrong cwd?
    - was because of build in console mode
