---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- We need to decide on the server software.
	- focus on creating a minimal api class 
		- whatever we plug there, whether dedicated serv instance or player hosted,
			- we'll just pull our own message format from there
			- We can as well add NAT punchthrough later
				- Games like soldat don't even have it
				- we'll read the guide on raknet even if we're not going to use it as it is comprehensive
	- what we need from the network
		- for early beta and later,
			- reliable delivery for initial stuff + chat
			- redundant delivery for inputs
				- we have to revive the reliable channel
			- perhaps compression?
		- for later stages,
			- masterserver + nat
		- for dedicated servers,
			- secure connections
	- we'll remove enet shortly
	- netcode.io + reliable.io
		- pro: we'll learn more along the way and have more control over packets
	- libyojimbo
		- pro: will probably work right away
		- it's actually for dedicated servers, not player hosted servers


- Editor-like server vars tweaker accessible by pressing ESC server-side
	- will have to be commandized properly, just like editor setup's

- Game events log and chat
	- Positioning based on input box window
		- Under the input box window, we can have tabs changeable by ctrl+tab and ctrl+shift+tab
	- Don't show input box window when chat is not active
	- Scroll can be added later
		- If we ever have scrollbar, change range, not coords.
	- Always show n recent commands. 

- we could begin by writing a simple chat server in order to test connections at all
	- we could revive our textbox because it was battle tested
		- actually, let's take imgui since we'll have tabs, collapsing etc for free
		- note we don't need selection of text, we'll just log entire chat history the simplicity if someone wants to copy
		- coloring could work by parsing actual content, instead of strangely structurizing commands
			- e.g. nicknames would always be colorized depending on the faction
		- wrapping will introduce a problem I guess but only the starting content will be colorized
			- yeah only the preffix will ever have colors
		- always wrap when inactive, but when active we can just as well wrap


- spectators
	- preferably only in the client setup

- what to send on client connection?
	- Note that WE WILL need SOME kind of reliable udp transport even for dumb chat messages
		- we'll use reliable.io or a hand-written slice/chunk proto
		- and like hell we are not going to retransmit them redundantly brute forcibly
	- for the initial cosmos solvable state, we can send a delta against the initial cosm and the current cosm
		- though entities for shells might get quite large
		- a character is almost 1k...
		- perhaps it will be best to send all inputs after all
	- if we want to replay either from session beginning or mid-way, inputs can become quite large
		- Though, we could use the exact same protocol
			- simply post all inputs for the client
		- and we could replay in parallel
	- cosmos solvable won't be that big
	- we also have to send rulesets if we'll allow commands on them

- it might be a good idea to split intercosm into many files?
	- in case of compatibility breaks we could only tinker with a single file
	- git has less to do as well

- check how openal behaves on windows when abruptly stopping sounds

- maps as git repositories
	- how do we facilitate modifications on existing maps so that they don't have to be re-downloaded?
	- we'd have to add remotes and assign branches to them

- always calculate the tickrate from the referential player in case the tickrate suddenly changes

- server_setup
- client_setup

- concept: server session
	- mode-agnostic sever vars
		- server_vars
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

- Remember to reset input flags of the character itself on performing transfer
	- why? it's good to keep shooting once you change

- Easily spawn loaded weapons or magazines
	- For now, let instantiation load them all by default

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

			
- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic
	- Seed will have to be sent in the beginning anyway, along with the state
	- Some amount of initial information will need to be transmitted anyway
		- Like current players?
		- Isn't this all a matter of sending the bomb mode state?

- matchmaking
	- stats persistence
