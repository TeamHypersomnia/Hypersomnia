---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- we can make the server always have the built-in functionality to play locally with a character
	- for barebones server, we can simply compile out the window, opengl and openal, even lodepng, leaving us with no-ops at all
		- server won't rely on image/sound information anyway, this will be the task of the mapper to once create a proper map
	- we can simply set a flag for whether we want audiovisual processing
	- menu guis won't be processed anyway

- plan for full server replays
	- server step != mode step?
		- server step tied to tickrate?
			- the cosmos and the mode anyway don't know about the rates, just of a delta to advance
	- it's just about saving the entropies
	- server shall be frozen and never advance when there are no players
	- keeping timing information in arena server vars
		- we will anyway have to commandize the changing of these rules somehow
	- server entropy different from mode entropy
		- since it will also store player information
	- server entropy serializer

- mode-agnostic sever vars
	- arena_server_vars
	- these actually can be called vars, why not

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

- document the offset picker

- adjust doppler factors of bullet trace sounds

- Remember to reset input flags of the character itself on performing transfer

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
