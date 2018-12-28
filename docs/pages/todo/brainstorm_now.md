---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- BETTER CLIENTSIDE PREDICTION
	- Battle events shall always be played referentially
	- We might want to predict sounds depending on the effect type
		- E.g. tie FoTA effects to the referential
		- But UWoTA to the predicted as there is a delay
	- We might want to give a context info for the start struct
		- This would hold the perpetrator id
	- Collision sounds
		- For now - predict all except collisions with remote players and their items
			- If a collider has a capability of a remote player - play in the past
			- otherwie always predict
				- Even throws should be fine
					- although some sounds might be unplayed if e.g. something is thrown closely to the wall
					- we could introduce a recent owner struct in item
						- if the recent owner is a remote player, play in the past
	- clashes
		- never predictable
	- deaths
		- never predictable

- Desync issues
	- Could it be that the predicted cosm is advanced by mistake instead of the referential one?
	- Perhaps operator= of the physics system inadvertently modifies something of the world which it copies?
		- The island only ever exists temporally.
		- Problem: the stack allocator is rewritten. There are possibly dangling addresses.
	

- Should parti/sound existence systems translate gunshot messages or shall we leave it to the post solve?
	- What if something is destructed?
	- Actually during gunshot nothing except cartridge is, and it is not taken into consideration

- fix Common state crash

- Admin panel
	- Editor-like server vars tweaker accessible by pressing ESC server-side
		- will have to be commandized properly, just like editor setup's
	- Should we re-use change property command?
		- we'll only need a dummy editor folder struct
			- we could make it more flexible

- Properly sync client's changes to nickname, sensitivity or others.

- Equipment generators
	- Should simply be markers that are used by the modes, depending on the flag
		- later we'll make the testbed conforming
	- is it important now?
		- i guess clientside prediction is more important
		- though we can plan for state

- Rebuy previous
	- Plan for state early
		- note that we don't need any bincompat regarding the mode solvable
		- because anyway we are importing/exporting without that state

- should rebuy previous also buy magazines bought?
	- perhaps

- Probably simply play win and death sounds in accordance with the referential cosmos
	- Will avoid confusion
	- Though will introduce lag
	- If, on the other hand, we want to predict deaths, it would be best if these were death sound entities
		- So that they get interrupted on mis-prediction
	- From what I can see, we already had some lag on the death sounds due to empty beginnings
		- Around 40-80ms
		- And we never noticed
		- So let's just always make death sounds referential

- fix arena gui showing "Disconnected"
	- somehow cache the nick or remove the entry?
	- this is cool actually but if someone connects right away it will show his nickname as the victim

- Watch out if fpset does not cause performance problems

- Do something so that we don't lose work in playtesting mode

- see if bilmer and vindicator have better intervals at 64hz

- Create randomized players like in the good olden times
	- to test the predicted experience
	- we might look into legacy sources for guidance
	- fill in several artificial connections starting from the back of the client array

- If we simply don't predict knockouts, we automatically don't predict the vulnerable win conditions in the mode
	- Other win conditions are based on time so it won't be as bad

- Notes on assymetric latency
	- Effectively, the client always shows AHEAD the server time by 
	- Therefore it is the client->server latency that is the most important
		- We should ALWAYS resend the earliest messages since traffic on this side will NOT be the bottleneck
		- so set the delay to 0 on client-side config

- Chosen solution for jitter buffer
	- Handling latency increase and thus, unaccepted client commands 
		- Client adjusts naturally, the same way as in the beginning of the play where latency is assumed to be 0
	- Handling latency decrease and thus, packet bursts
		- Two strategies
			- One, squash on the server 
				- Pro: simpler, so we'll for now go with this strat
				- Pro: gets the client fastest on track
				- Con: slight jerks when this happens
				- Was the same not mentioned with snapping of the ticks?
			- Second, slow down tickrate on the client and thus the rate with which the commands are generated
				- Pro: no jerks when this happens
				- Con: takes more to get the client faster on track

- Server: accepting inputs
	- I guess a simpler jitter buffer implementation could be in order
		- e.g. just keep a vector and a maximum of steps to squash at once?
	- jitter protects from latency increase, squashing from decrease

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

- Step configuration for the cosmos
	- Whether to process deaths, e.g. to never predict them on the client
	- Whether to post audiovisual messages, always false for the server

- Implement steps correction sending 

- Chat-level logs
	- server_setup has to expose events somehow
	- can send them really as chat messages to all the clients
		- we also need to redirect it to the server player

- Sending large step infos through yojimbo?
	- we probably want to handle it after DM milestone
	- Don't rely on fragmentation

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
