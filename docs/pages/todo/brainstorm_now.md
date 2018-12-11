---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Step configuration for the cosmos
	- Whether to process deaths, e.g. to never predict them on the client
	- Whether to post audiovisual messages, always false for the server

- Client-side
	- When initial state is received, wait for the first entropy
	- When it arrives, simply begin queuing inputs localy
	- The server only sends a "client input accepted" byte in response
		- when this happens, peel off the oldest input from out queue
	- always re-simulate all inputs in the queue
		- we don't have to calc some difference, this will happen naturally
	- how does this approach scale when the effective latency suddenly decreases?
		- so a server suddenly gets a burst of packets from the client
			- if we unpack them evenly into steps, we don't decrease the effective latency
		- since some steps were missed, now we have to squash inputs
			- server sends a number of how many inputs were squashed?
		- don't worry, by definition squashing will only occur in high-jitter environments
			- squashed entropies should still preserve important behaviour
			- e.g. you won't be left with a gun that shoots even though you've released the button already
			- magnitude of movements might be malformed so we'll have a hitch, though nicely interpolated
	- how does this approach scale when the effective latency suddenly increases?
		- client just doesn't peel off inputs for a while from its queue
	- to avoid squashing as much as possible we can have a server-side jitter buffer for clients
		- though I remember we didn't have some good experience with it
	- if the client input was not accepted, still peel off the back queue!
		- simply treat it as a misprediction!
		- well, this sucks, because we can possibly miss some important inputs like a button press
		- suddenly our player stops moving!

- It would be nice if the server_setup could only accept ready structs and was not concerned with messages being preserialized, 
	and serialization in general
	- similarly as it receives "connection event" message
	- let each message declare using payload_type
	- for reading, the payload type would be created on the stack in the adapter, and then constructed by serializators from a separate header
		- server would receive a ready reference to a struct
	- it would be best if the server_setup controlled whether the payload type is to be on the stack or if it's ready somewhere else
		- read_into
		- instead of a lambda, have a template function handle_message which accepts a payload type as its argument
			- then pass *this to the server advance
			- the callback also accepts a callback for reading into so we control if the payload type enters stack or somewhere else
				- if we decided that the yojimbo messages should hold the payload directly, the server adapter can call std::move in its lambda passed to the message callback
		- what about structs that its not comfortable to mash into a single struct but still need to be read?
			- e.g. initial solvable has multiple big objects in a single message
				- mode solvable
				- cosm solvable
				- server vars
			- do we send them separately?
				- feels clean but possibly lost opportunity for compression
					- although these things won't compress much better in conjunction
				- we also get more latency due to separation of blocks
			- read_payload accepts several arguments for specific types, we pass the message type as the type
				- Indeed, it is us who decide whether to pass more arguments
				- we can add a type list as a payload type
				- this kind of thing will happen on the client though

- Whether to keep worst-case storage in the preserialized messages or not is another problem
	- I guess tlsf will be efficient here

- It might be hard to properly send the initial mode state while avoiding desync
	- sending solvable on init is also scalable for longer matches
	- Rounds might be restarted arbitrarily due to pre-solve logic
		- e.g. game commencing
	- and when that happens the cosmos solvable is already altered

- Optimize for bandwidth later. 
	- For proof of concept, it will be enough to brute-force-write the inputs with our own serializers.

- Connection helper interface?
	- so that we have same interface for client and server and dont repeat ourselves
	- we don't have much of these public funcs, though
	- and it would probably complicate matters needlessly
		- even pro yojimbo doesnt to that

- Notice that serialization of mode entropy will be a lot more complicated on the server
	- So, we will have completely separate funcs for read and write
	- We might later write player entropies from the one that posts the most messages
		- we will check the bounds for the message anyways and drop later entropies for now
	- E.g. we will calculate client ids by looking for them in the modes

- Properly send initial state on connection
	- Serialize right away and hold std::vector<std::byte> in the message structure
		- In particular, dont create deep clones of the solvable or entropies 
		- That's because serialization will be a lot faster than deep clones
		- And we anyway have to serialize this so that's one step less

- Implement steps correction sending 

- General client processing loop
	- Always check and add a mode player if the client is not yet in-game
		- Handles restarts automatically
	- if we do it per step, there will be a delay for the state to catch up, but:
		- State is simpler
		- We don't have to worry about malicious sizes
		- We don't have to worry about message being too large due to all these nicknames
		- Con: we might waste more bits/bytes overall but that's amortized over time

- Chat-level logs
	- server_setup has to expose events somehow
	- can send them really as chat messages to all the clients
		- we also need to redirect it to the server player

- Client FSM
	- PENDING_WELCOME 
	- RECEIVING_INITIAL_STATE
		- after receiving initial state, we might want to send inputs that happened as a block as well
	- IN_GAME
	- Last time of valid activity
		- Kick if AFK
		- AFK timer can already be a server setting

- The built-in player of the server
	- Always at id -1
		- mode always allocates maxmodeplayers + 1 in its array
	- The server setup will thus hold entire intercosm

- Preffix the single client entropy with a byte.
	- The first bit signifies whether it is cosmic or mode
		- we will only allow one type per step to increase number of message types possible within remaining 7 bits
	- Remaining 7 bits will signify whether there's data for:
		- cast spell
		- wielding
		- intents
		- mouse
		- etc.
		- for mode: item purchase, team selection etc.

- Properly sync client's changes to nickname, sensitivity or others.

- Sending large step infos through yojimbo?
	- we probably want to handle it after DM milestone
	- Don't rely on fragmentation

- Let's just create a proof of concept using yojimbo, don't focus on creating a minimal augs interface
	- When it works, we'll gradually abstract it away and see if it still works

- Will yojimbo handle 128hz?
	- We can send packets once every second tick, so at 64hz

- if we don't send any message at all for when a step has no entropy,
	- we CANNOT possibly advance the referential cosmos in time
		- which might become problematic once we have to simulate it forward several seconds
	- so we should always send at least empty step message

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

- Delta compress the solvable to send against the initial solvable

- check how openal behaves on Windows when abruptly stopping sounds

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
