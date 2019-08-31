---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- less delay for uwota
- fix triad missing explosions

- We still have crashes on Windows, even after fixing swap buffers
	- Perhaps it's a compiler bug since with our home-brewn build everything seemed to be fine
	- Although I think it was the same with that mega.nz build

- Inventory GUI still acts up


- Balance grenades
	- +0.1ms delay
	- A little increase in price

- ask glenn about those tokens

- do_server_vars
	- both in settings gui cpp and in rcon
	- just simply apply & save buttons along with revert
	- these changes are too important
	- we can easily make snapshots of vars applied

- Ruleset chooser might be a combo too
	- it might only read the names or ids from the ruleset file
	- rcon can set values on the go (later)
	- or load a lua (later)

- Re-transmission of server_vars as a universal rcon feature
	- Handles map changing
		- How do we load arena?
		- Do we deterministically re-insert all clients?
			- Probably 

- Update imgui later for cool features like builtin tabs

- Maybe allow map changing through rcon so that we could switch to this other map?

- Map transmission
	- Maybe asynchronously compress the entire map to .7z

- Maybe a separate folder for official arenas?

- Disallow exporting to lua when playtesting

- LPM acts as Q when hands are empty
	- Punches when no weapons
- RPM could always punch

- Write autoupdater so we don't have to save those avatar images in temporary
	- We'll keep them in cache, just autoupdater will preserve some config values
	- We might keep the entire config local lua intact, just per-push specify what config values to overwrite

- DO NOT disclose a single rcon password to multiple moderators/admins
	- That is because a single malicious RCON holder could avoid responsibility for their misdeeds
	- Instead, just assign permissions to accounts, or just the private IDs

- Check if we don't have some dump file on windows

- Do we want to somehow let the user be able to rollback to older versions?
	- How do we even do it? Build retention on appveyor?
	- Not now, certainly not until editor is publicly usable

- Check if there's no room for error in those connection tokens

- Dedicated server communication
	- Advanced RCON functionality
		- Rcon can download the log
		- Rcon should have a reserved slot to enter in case of emergency
		- RCON password
		- Switch teams
		- Restart
		- A way to view dedicated server stats?
	- Remember to keep the old master rcon password so that basic level rcons cannot change it

- Automatic updater?
	- Would also solve the problem with cache?

- Hold user avatar/nickname/other identity in appdata folder
	- so that we don't have to specify each time a new version comes out

- We could set a limit to the number of allowed simultaneous muzzle sounds from the same gun
	- similarly for health decrease sounds

- Remember to re-import the new content to cyberaqua after we're done

- We might want to somehow decrease heap contention between threads
	- Best would be per-thread heaps
		- Even better just no allocations
		- Though even something as trivial as draw debug details will do a lot of allocations
	- Use hoard allocator?

- Benefits of demos
	- Deterministic repros
	- Can record without performance hit
	- Can later record in highest quality only the highlights
	- Fun moments will never be lost

- We can automatically record demos for every server session
	- Demos could just be network messages applied at step x
	- We could pretty much resimulate the entire client setup this way, just without sending messages
	- Also makes it easier to debug
	- Snapshots could prove a little hard but we could just resimulate from the beginning if we want to seek backwards

- Port fy_minilab
	- though change "invariants" to "invariants_state" and same with components

- Upgrade appveyor
	- Things to update for windows
		- Appveyor script: llvm 8 when the visual studio 2019 is ready to take the new filesystem
			- We had differing versions, the one on our computer is newer so it builds everything
		- augs::date_time constructor for file write type, used to display last write times of lua meta files, although that's low priority

- blurred text if zoomed out

- fix area indicator zoom in editor

- In-game tip system
	- Notifications like "can't holster" will be drawn a little above the context tip
		- So that both can appear at once

- Keep timestamps in log structures and, when writing the logs to a file, preffix the log entries with time
	- Will later be good to separate logs via date, for the dedicated server

- Dump logs once every 1000 or so
- Write editor write date to version.txt file
	
- Radar
	- Also show bomb

- make layer with insects hoverable in editor

- bomb falls outside the map


- Indeed, there is a problem when importing project-specific gfx on windows, but not on linux

- fix neon maps being generated in the project folder

- Note that message buffer might overflow during resynchro, causing a disconnection

- increase prices of uwota and triad? theyre soo op

- do something about going through walls with bilmer and elon hrl

- check if export/import of rulesets works correctly

- when post-solving referential, one could see if a similar-enough effect has happened in the predicted post solve.
	- if not, we want to post it, because a predicted cosmos might have not predicted this effect occurring.
	- this could be done for id-insensitive events like effects tied to weapons and characters
	- and not necessarily for bullets 

- there was a desync on win 8.1

- why would a warx fq12 be reloaded?
	- is another mag chosen for it?
	- do we use strict greater inequality to acquire the better mag?

- fix client being unable to reconnect
	- the effects are being made unpredictable due to being thrown

- when re-exporting
	- fix spells
	- remnants
	- weapons
	- character stats

- Bug: path specification for assets doesnt fully work on Windows

- in editor, allow modifications after re-export
- in exported flavours, identify by filenames and flavour names not by ids

- during akimbo, only drop when the G is released, not right away when it is pressed
	- when G is still held, you can press either LPM or RPM to decide which weapon to drop
	- if G was released without holding lpm or rpm, drop the most recently wielded item as always
	- if G is still held while we have only one item left in hands, still allow to drop by pressing either LPM or RPM  

- in case the gui still acts up
	- always keep this personal deposit open
	- recalculate hotbar on every round start?

- could chambering sound be omitted under some occasions?

- particles don't get properly predicted on deaths sometimes?

- if the referential post solve determines that the predicted post solve has missed something predictable (e.g. a bullet impact)
	- we should somehow try to replay it

- Fix prediction of collision sounds
	- Never predict collisions of remote players
		- This might be important for not exposing tactical information
	- Predict collisions with items only if they weren't just recently dropped by a remote player
	
- test O3 with and without flto?
	- Could save us much of the build times for production

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
		- due to id collision
		- maybe store nickname?

- Do something so that we don't lose work in playtesting mode

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

- Delta compress the solvable to send against the initial solvable

- check how openal behaves on Windows when abruptly stopping sounds

- maps as git repositories
	- how do we facilitate modifications on existing maps so that they don't have to be re-downloaded?
	- we'd have to add remotes and assign branches to them

- always calculate the tickrate from the referential player in case the tickrate suddenly changes

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
