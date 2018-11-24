---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- send knives flying the other way when dropped due to a knockout

- define music as always longer than x and always hearable

- what to send on client connection?
	- if we want to replay either from session beginning or mid-way, inputs can become quite large
	- cosmos solvable won't be that big
	- we also have to send rulesets if we'll allow commands on them

- it might be a good idea to split intercosm into many files?
	- in case of compatibility breaks we could only tinker with a single file
	- git has less to do as well

- check how openal behaves on windows when abruptly stopping sounds

- maps as git repositories
	- how do we facilitate modifications on existing maps so that they don't have to be re-downloaded?
	- we'd have to add remotes and assign branches to them

- roundsounds and warmup themes
	- we could introduce a separate file for ruleset viewables
		- simply in mode GUI, use a flag for the unpathed/pathed asset widget that determines which viewables to write to
		- alternatively, since viewables are never too big, simply always have viewables separate from the intercosm
			- good for skinning the intercosm without ever having to modify it
	- it would be nice if the sound system could synchronize music against the current arena state
		- though just warmup. no need to synchronize roundsounds which are short by nature
			- specifying the warmup theme sound
				- chosen solution: flavour
					- best performance
					- low flexibility
						- customization without altering the intercosm would involve changing the viewable definition
						- might be necessary to introduce another warmup sound decoration entity flavour
					- lowest coding complexity
					- also lowest design/explaining complexity, not just coding
						- we might be dead before the need for a more complex solution arises
					- a bool for whether the music position should be synced?
						- or a float with seconds tolerance
							- although that would preferably be the client space setting
				- sound effect input, spawn entity with unique sound
					- 'might' incur performance problems?
					- best flexibility
					- moderate coding complexity
					- still two solutions: 
						- a new entity type for unique sound, or
							- even worse compilation times...
						- add a component unique_sound_effect to the existing type
							- memory overhead
				- sound effect input, sound system infers information from the mode
					- best flexibility
					- best performance
					- worst coding complexity - greatest coupling
		- instead of having to create entities? what if we make that functionality cosmos-specific?
		- there could be a music entity that plays globally
		- it would also be nice to have themes in rulesets?
			- well we anyway have viewables definitions in the intercosm so...
			- in practice every alteration in roundsounds will require a different map 
				- and later a rebase, wtf
				- rebases won't be possible with binary files

- always calculate the tickrate from the referential player in case the tickrate suddenly changes

- arena_server_setup
- arena_client_setup

- concept: server session
	- mode-agnostic sever vars
		- arena_server_vars
			- perhaps they should not AT ALL influence the deterministic state itself
				- problematically, logic_speed_mult does influence this state by influencing the requested delta
				- so does the tickrate though
					- although that should only generate entropy
			- max players? 
			- okay why not simply have both the tickrate and the audio speed in arena mode ruleset?
				- simplest solution, not pure semantically but still architecturally somewhat
				- we'll still have *some* server vars
			- basically they should only change how entropy-generating code behaves and the timing code
			- timing info
				- tickrate
				- snap audio speed to simulation speed
		- these actually can be called vars, why not
	- server step != mode step?
		- server step tied to tickrate?
			- the cosmos and the mode anyway don't know about the rates, just of a delta to advance
	- server step will also be passed to the packed
		- only ever zeroed-out on hard server resets OR when there are zero players?
		- server step does not have to correlate to the cosmos step at all
			- in fact they will most of the time be different due to rounds being reset
			- so it's fine to always zero-out the server sequence when there are no players
	- we can make the server always have the built-in functionality to play locally with a character
		- for local plays, it won't be much of an overhead
			- it doesn't even have to be used for pure local setups
			- can be used for the editor playtesting to have full test experience?
				- actually even then it's not required
		- menu guis won't be processed anyway
		- for now we don't need a barebones server
			- we'll test it when the time comes
			- for barebones server, we can simply compile out the window, opengl and openal, even lodepng, leaving us with no-ops at all
				- server won't rely on image/sound information anyway, this will be the task of the mapper to once create a proper map
				- we can also simply set a flag for whether we want audiovisual processing
				- also completely compile out the vws 

- plan for full server replays
	- it's just about saving the entropies
	- server shall be frozen and never advance when there are no players
	- keeping timing information in arena server vars
		- we will anyway have to commandize the changing of these rules somehow
	- server entropy different from mode entropy
		- since it will also store player information
	- server entropy serializer


- Letting servers adjust the speed of the game
	- bomb mode doesn't do timing, it just advances whenever asked, but it has to effecctively use the delta information
		- which is obtained by lsm / tickrate
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

- adjust doppler factors of bullet trace sounds

- Remember to reset input flags of the character itself on performing transfer
	- why? it's good to keep shooting once you change

- Easily spawn loaded weapons or magazines
	- For now, let instantiation load them all by default

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- Game events log and chat
	- In the same window
	- ImGui or own GUI?
		- We actually have some textbox code we can introduce later for chatting
			- Better control over such an important feature
			
- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic
	- Seed will have to be sent in the beginning anyway, along with the state
	- Some amount of initial information will need to be transmitted anyway
		- Like current players?
		- Isn't this all a matter of sending the bomb mode state?
