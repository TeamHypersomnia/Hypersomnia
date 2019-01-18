---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---


- fix ctrl+v on windows
- bug: after knockout, some weapons remain tied to the player
	- this only happens in online play

- check if editor now launches in windows release build (not relwithdebinfo)

- when re-exporting
	- fix spells
	- remnants
	- weapons
	- character stats

- Apparently, we still get desyncs even with a reproducible float library.
	- check out these reinferences because maybe desyncs are due to them
		- stress testing needed

- increase range of secondary throw
- check if performance problems persisted in older versions, those played with kartezjan and before repro math

- the personal depo still gets closed sometimes
- prefer low dampings
- maybe allow running for longer as it is a running heavy game
- prevent trigger pull sound during chambering
- reinferences should NOT occur at all during play because we are cloning	
- log if client does not have the map that's on the server

- walking around many shells slows down so it's probably due to logic

- always hide the weapon first if reloading
- disable buying in warmup
	- or just 0 money

- arena gui predicts wrong
	- draw deaths only for referential, i think it might be the other way around
	- problem is, it draws based on state not on events?
		- but the referential state for the bomb mode should properly have the kill

- knives should not be thrown if wheelup is pressed during freeze
- can probably buy when dead
- the problem still persists with weapons being glued to someone
- freeze someone on connection if they connect

- fix those dangling particle streams as they may give away positions 

- Bug: path specification for assets doesnt fully work on Windows

- allow modifications after re-export

- in flavours, identify by filenames and flavour names not by ids

- write to streflop dev about that copysign should be bivariate

- during akimbo, only drop when the G is released, not right away when it is pressed
	- when G is still held, you can press either LPM or RPM to decide which weapon to drop
	- if G was released without holding lpm or rpm, drop the most recently wielded item as always
	- if G is still held while we have only one item left in hands, still allow to drop by pressing either LPM or RPM  

- in case the gui still acts up
	- always keep this personal deposit open
	- recalculate hotbar on every round start?

- check windows dll dependencies for determinism

- Desync issues
	- Resources:
		- https://techdecoded.intel.io/resources/floating-point-reproducibility-in-intel-software-tools/
		- https://software.intel.com/en-us/articles/getting-reproducible-results-with-intel-mkl
		- https://scicomp.stackexchange.com/questions/24423/small-unpredictable-results-in-runs-of-a-deterministic-model
			- Multithreaded computations on parallel cores. Modern computers typically have 2, 4, 8, or even more processor cores that can work in parallel. If your code is using parallel threads to compute a dot product on multiple processors, then any random perturbation of the system (e.g. the user moved his mouse and one of the processor cores has to process that mouse movement before returning to the dot product) could result in a change in the order of the additions. 
			- I think this is only a problem with Intel MKL
		- https://software.intel.com/en-us/forums/intel-c-compiler/topic/518464
		- https://www.hemispheregames.com/2017/05/08/osmos-updates-and-floating-point-determinism/
		- https://stackoverflow.com/questions/20963419/cross-platform-floating-point-consistency
		- http://nicolas.brodu.net/en/programmation/streflop/index.html
			- Even then, the IEEE754 standard has some loopholes concerning NaN types. In particular, when serializing results, binary files may differ. This problem is solved by comparing the logical signification of NaNs, and not their bit patterns.
			- ALignment of data and vector instructions. Modern Intel processors have a special set of instructions that can operate on (for example) for floating point numbers at a time. These vector instructions work best if the data is aligned on 16 byte boundaries. Typically, a dot product loop would break the data up into sections of 16 bytes (4 floats at a time.) If you rerun the code a second time the data might be aligned differently with the 16 byte blocks of memory so that the additions are performed in a different order, resulting in a different answer. 
				- Fortunately for us malloc aligns to 16-byte boundaries for us
				- what about static allocations?
		- https://stackoverflow.com/questions/26497891/inconsistent-float-operation-results-between-clang-and-gcc
		- http://christian-seiler.de/projekte/fpmath/
		- use openlibm or crlibm?
		- use fp strict on linux as well, there's a switch
		- ensure that fpmodel holds every frame
			- actually we do already 
			- the guy on Box2D forum checked only that nearest mode and something else?
				- It was _MCW_PC which is unsupported on x64 per the msdn docs and intel forums here:
					- https://software.intel.com/en-us/forums/software-tuning-performance-optimization-platform-monitoring/topic/277738
		- https://stackoverflow.com/a/26502092/503776
	- flags to think about
		- ffast-math
	- a client receives a state at N and does a complete reinference on it
	- the entropy for N+1 contains a "player added" entry which forces a complete reinference
	- shikashi, the clients did not reinfer state at N which the client has received
	- order of collisions happening?
		- probably not
	- Could it be that the predicted cosm is advanced by mistake instead of the referential one?
	- Perhaps operator= of the physics system inadvertently modifies something of the world which it copies?
		- The island only ever exists temporally.
		- Problem: the stack allocator is rewritten. There are possibly dangling addresses.
	- There is a case where Data and Shuncio were synchronized with each other, but both had non-canonical version of the world, while I had it canonical
	- possibly bad packet payload?
		- why would a client on the same machine not desync?
	- turn off fused add and multiply
	- test if replaying editor session yields same results if forcing a re-serialize of the solvable every frame
		- and if serialized bytes are identical after re-serialization cycle
		- make sure to use Release on Windows
		- we can re-serialize random number of times to avoid the need for relaunch
			- sometimes 0
	- to test reinference determinism, test if completely reinferring arbitrary number of times yields equal result as reinferring always just once
	- perhaps reinference inadvertently modifies something of the solvable?
		- however, everybody reinfers all the same
	- doesnt the joined player reinfer twice? once on load, another time on shall reinfer

- 2 seconds longer freeze time

- (Shuncio) some possible crash before start of the next round

- knockouter is dead if someone disconnects after having shot and that bullet kills another player
- could chambering sound be omitted under some occasions?
- bomb soon explodes theme gets repeated
	- unimportant: perhaps it was 

- particles don't get properly predicted on deaths sometimes?

- if the referential post solve determines that the predicted post solve has missed something predictable (e.g. a bullet impact)
	- we should somehow try to replay it

- increase max predicted from 130 to more 

- We were considering whether to mark the bomb as never predictable,
	- however, the worst that could happen, is that we mispredict that we are dead due to somebody shooting us
		- whereas we are actually alive and we have defused the bomb
		- either way the death impact would be predicted which would be misleading

- Fix prediction of arena gui

- Fix prediction of collisions
	- Never predict collisions of remote players
		- This might be important for not exposing tactical information
	- Predict collisions with items only if they weren't just recently dropped by a remote player

	
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
