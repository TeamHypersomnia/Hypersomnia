---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- general mode command list
	- don't make a namespace for them
	
	
- custom factory in separate file to not mess up formatting
	- simply cast int to type index in type_in_list_id
	- type_list with all message types
	- then dispatch
	- then we dont need the enums whatsoever and write a simple wrapper in net adapter for creating messages passing the type once

- Properly send initial state on connection
	- Serialize right away and hold std::vector<std::byte> in the message structure
		- In particular, dont create deep clones of the solvable or entropies 
		- That's because serialization will be a lot faster than deep clones
		- And we anyway have to serialize this so that's one step less
	- For now, let's only send inputs per step instead of solvable
		- This way we always quickly connect on the beginning of the round which is most frequently the case
		- if you connect late that's your problem, but anyways it should happen rather quickly
		- we can later implement dynamic choice of the way we transmit the initial state
			- solvable's can be trivially estimated
			- and we can keep track of the number of sent entropies this round, as we go
			- this way we can compare which is more space efficient
	- research the completion checking of messages in yojimbo
		- we can use HasMessagesToSend
			- actually not really because its hidden behind a private interface...
		- Use the reference count, simply AcquireMessage on being posted
			- this is even more flexible since it works even if we post more messages after this one
		- perhaps by message id or a changed field value inside the message object?
	- actually why do we need completion? can't we begin to queue up the inputs?
		- we want to generate another solvable message right away or pack all inputs that happened tightly


- General client processing loop
	- Always check and add a mode player if the client is not yet in-game
		- Handles restarts automatically
	- if we do it per step, there will be a delay for the state to catch up, but:
		- State is simpler
		- We don't have to worry about malicious sizes
		- We don't have to worry about message being too large due to all these nicknames
		- Con: we might waste more bits/bytes overall but that's amortized over time

- commandize add_player and remove_player
	- problem: we might be unable to deterministically predict the outcome
		- what? actually, add player command will always succeed as long as there is space, which we can easily predict
		- we won't predict auto-assignments but that's not really important
		- also if we need predictability, wwe can do these once per step
	- remember that we need to re-add players every time that a mode type is changed

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

- Let someone on spectator when they are connected even if they're downloading initial solvable
	- Though before they git clone the repos
		- Not before we set up the masterserver.
			- That is because clients will know about the map to be downloaded only through the masterserver that will expose the details of all of its servers.

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

- server entropy
	- chosen solution: a client should have no notion of other clients.
		- Therefore, add/remove player should be handled by MODE ENTROPY.
		- Mode entropy is anyways serialized to all players.
		- for now it's not critical that we serialize server messages, and their handling will anyway be centralized so we can introduce it later
			- therefore for now just handle everything inside the run function with callbacks
	- Surely: client connected, client disconnected, variant of messages
	- maybe not for the sake of replaying, but for the sake of syncing non-step-entropy information to the clients?
		- like connections/disconnections
		- though a part of it might be private to the server?
			- not really, I guess authentication would be transparent to the setup and handled by web backends and netcode io
	- could it be made to be processed by clients as well?
		- since the server also needs a way to communicate to others that a client has just connected
	- replicated_state
		- current cosmos
		- current mode variant
		- server rules
	- network_game_interop
		- an intermediate object that deterministically plays the effects of network entropy upon a mode
		- advance
		- responsible for adding/removing players to/from the modes
			- perhaps application of other messages
		- after server constructs the total server_entropy, with total inputs of players... 
			- now the problem is that the input calculation is transparent for replays
				- though it is a trivial thing

decides which ones to send to the clients so that they don't 

- perhaps we should implement remote entropy and just return it from the adapter, instead of acquiring templates
	- because we'd anyway like to log the entropies
	- unless we log entropies at the packet level

- network_adapters
	- has usings for server/client types
	- global functions to abstract functionality?
		- actually just use a class wrapper and a getter for the specific object so that we know where we're getting specific

- Sending large step infos through yojimbo?
	- we probably want to handle it after DM milestone
	- Don't rely on fragmentation

- Let's just create a proof of concept using yojimbo, don't focus on creating a minimal augs interface
	- When it works, we'll gradually abstract it away and see if it still works

- Will yojimbo handle 128hz?

- if we don't send any message at all for when a step has no entropy,
	- we CANNOT possibly advance the referential cosmos in time
		- which might become problematic once we have to simulate it forward several seconds
	- so we should always send at least empty step message

- what if reliable messages in yojimbo are actually redundancy?
	- They are if we configure them like so.

- improvements to redundancy
	- message-wise fragmentation
		- we might need some more effective way to deal with this
			- Solution 1
				- if there are 5 steps to sent, but 2 steps already fill the whole packet,
				- and the other 3 can be packed into the next,
				- simply withhold posting the 3 remaining messages until the other 2 are delivered
				- in this case we might need to increase the rate of sends and send them even before the next tick
			- Solution 2
				- simply always send all messages redundantly but split into several packets
				- client, apart from sending the most recent sequence number before which everything was received,
					- also sends acks redundantly for all packets received out of order
						- clears them once the most recent sequence number implies receiving of these out of order packets as well
				- con: we would probably want to prioritize the 

		- we have a map of sequence to reliable range
		- later entries may arrive before earlier ones
		- we keep a vector of pending entries and only  
	- Unlikely, but a single step information might not fit into a packet
		- e.g. if 300 players move their mouse at once
		- in this VERY unlikely case we might just randomly drop some entropies to not overload the server?
			- simply drop entropies of players who've sent the most commands
				- so if someone coordinates a DoS of this sort, they will remain motionless
				- if someone is innocent but their entropy is dropped, it might happen that:
					- their player moves despite having their buttons pressed
					- their mouse position is completely different than expected
				- in this case, the client should try to repost the entropies that were cut

- large block sending augs api
	- asynchronous: deliver_large_data
	- large_data_delivered() const
	- always one large data is sent at a time
	
- solution: netcode.io + sending large blocks code from the example,
	- or we'll take some code from yojimbo that sends blocks

- against yojimbo?
	- the message ecosystem that does not play well with our code
	- other deps that we don't need atm

- any cons for using netcode io?
	- perhaps bandwidth overhead for encryption, but should be negliglible?
	- might be a hassle to setup this in cmake but should be doable

- problem: reliable.io has no concept of messages, we will have to learn how it is used in yojimbo or just somehow figure it out
	- does reliable.io even do what we think it's doing? e.g. sending large blocks of data. This is the only thing we need it for.
		- actually, yojimbo can do it

- our bidirectional flow of inputs will be based on redundancy, 
	and we'll also need some fragmentation logic

- design augs api so that either netcode or yojimbo can be used underneath

- PIMPL anywhere?
	- actually netcode.io headers are pretty damn light

- Besides encryption (which we can add later), what do we want from netcode.io?
	- connection management against trivial ddos attacks, I guess

- For now let's go with winsock + reliable.io to send large blocks
	- We'll learn from netcode and maybe use some functions
	- API will never need to change when we add security.

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
