---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Procedure for the game server
	- Once every 5 seconds, send statistics line to the masterserver
		- Contains external/internal address:port info
		- num of players and all these stats
	- Masterserver smoothly registers the server every time it encounters this
	- If no messages came for a minute, delete the server from the list
	- Mapping by the external ip
	- To send stats, use the socket already bound in yojimbo server, somehow extract it
	- Responding to ping requests
		- A potential game client will ping this server
		- We only need to send a ping back
	- It looks like we'll never send stats on-demand; only periodically - completely on our terms
	- so netcode only needs a ping packet

- Sending server stats
- Actually it's better if stats come from the same port because nat will be less complex

- Multiple official servers
	- Picking best
	- Do this once we have pinging implemented, which we want with the list of community servers

- Masterserver
	- For now, we'll update it manually whenever the need arises
	- NAT punchthrough builtin
	- Connect on Browse

- Website server status
	- Game servers have a built-in http server?

- Check official map version upon connection
	- if mismatch
		- custom? re-download
		- official? disconnect

- Leaderboards
	- Columns: Avatar, Nickname, Kills, Assists, Deaths, Hours played

- Setup a simple dev journal at hypersomnia.xyz

- Advanced RCON functionality
	- Kicking and banning users
		- Rcon can download the log
		- Rcon should have a reserved slot to enter in case of emergency
		- Restart
		- A way to view dedicated server stats?

- Advanced demo replay functionality 
	- Optionally be able to add a synchronized audio track for scaling with the time
		- If we recorded the voice separately for example
		- It will synchronize with slow motion etc
	- We can make non-serialized snapshots easily
	- Autodirector
		- Minimize the time we have to spend on editing the footage
		- Highlights of each 10 second durations preceding ends of rounds
			- for compact duel of honor recordings
		- Smooth slow-motion seconds setting in gui
			- augs::maybe
		- Somehow detect action?
			- Or just show the player that is going to be killed next?
	- next/prev round and death
		- textbox with an offset for deaths
		- 5 second threshold for rounds
		- demo replay advance takes a lambda 'has_occured' that takes an enum demo_occurence_type KNOCKOUT/ROUND
		- alternatively steppers may return some metadata with a bitset of occurences that happened, if it depends on post-solve
			- but we can look into knockouts

- Demo replay fixes
	- Money is wrongly displayed on demos
		- I guess it is shown for the the local character
		- we should also show all moneys on the scoreboard
	- Reset timer after seeking during play, because lag will force to advance again

- Let shells not invoke any sound when hit by a grenade
	- to lessen hrtf impact and occurences of sound interruptions

- Update process fixes
	- On update on linux, symbolic links to filesystem handlers are lost

- Advanced update automatization
	- CLI for editor re-exporting
	- Server updates
		- Check once every X minutes or everyday at a predefined hour
		- Remember to only pull an upgrade if both Windows and Linux versions match
		- For now have a rcon command for "Schedule server update"
	- While in the main menu, check for updates once every several seconds
		- e.g. if the server has to restart due to an upgrade, the clients will follow with the update right away
		- also trigger update check every time we enter the main menu from the client
		- New version available!
			- Do you want to restart and upgrade the game now?
				- Upgrade
				- Cancel
					- Automatic update was cancelled.
	- Community map conversion

- Faction win sounds sometimes get spatialized, when switching spectated person

- Spectator fixes
	- Don't force spectated player switch upon match summary
	- Fix spectator sizing after death
	- Show death summary in spectator

- Run thread sanitizer at least once

- Fix openssl errors in build process on a clean arch linux
	- Perhaps just use shared libraries if shared can't be found

- Chat messages bugs?
	- Research chat messages sometimes not working
	- Fix too many chat messages

- Don't kick afk spectators
	- send some heartbeat

- Inventory GUI still acts up

- Test what happens without internet connection when launching Hypersomnia on Windows

- Font-scale invariant update window

- Website

- custom chosen ruleset could be a part of mode solvable
	- or actually client/server solvable (arena_handle)

- Ruleset chooser might be a combo too
	- it might only read the names or ids from the ruleset file
	- rcon can set values on the go (later)
	- or load a lua (later)

- Update imgui later for cool features like builtin tabs

- Community map transmission
	- Maybe asynchronously compress the entire map to .7z

- Disallow exporting to lua when playtesting

- Gameplay fixes

- Gameplay bugs
	- The problems with walls
		- Can walk through
		- Can shoot through

- LPM acts as Q when hands are empty
	- Punches when no weapons
- RPM could always punch

- DO NOT disclose a single rcon password to multiple moderators/admins
	- That is because a single malicious RCON holder could avoid responsibility for their misdeeds
	- Instead, just assign permissions to accounts, or just the private IDs

- Do we want to somehow let the user be able to rollback to older versions?
	- How do we even do it? Build retention on appveyor?
	- Not now, certainly not until editor is publicly usable


- We could set a limit to the number of allowed simultaneous muzzle sounds from the same gun
	- similarly for health decrease sounds

- Remember to re-import the new content to cyberaqua after we're done

- We might want to somehow decrease heap contention between threads
	- Best would be per-thread heaps
		- Even better just no allocations
		- Though even something as trivial as draw debug details will do a lot of allocations
	- Use hoard allocator?

- blurred text if zoomed out

- fix area indicator zoom in editor

- In-game tip system
	- Notifications like "can't holster" will be drawn a little above the context tip
		- So that both can appear at once

- Keep timestamps in log structures and, when writing the logs to a file, preffix the log entries with time
	- Will later be good to separate logs via date, for the dedicated server

- Dump logs once every 1000 or so
- Write editor write date to version.txt file
	
- make layer with insects hoverable in editor

- bomb falls outside the map

- Note that message buffer might overflow during resynchro, causing a disconnection

- increase prices of uwota and triad? theyre soo op

- check if export/import of rulesets works correctly

- when post-solving referential, one could see if a similar-enough effect has happened in the predicted post solve.
	- if not, we want to post it, because a predicted cosmos might have not predicted this effect occurring.
	- this could be done for id-insensitive events like effects tied to weapons and characters
	- and not necessarily for bullets 

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

- particles don't get properly predicted on deaths sometimes?

- if the referential post solve determines that the predicted post solve has missed something predictable (e.g. a bullet impact)
	- we should somehow try to replay it

- Fix prediction of collision sounds
	- Never predict collisions of remote players
		- This might be important for not exposing tactical information
	- Predict collisions with items only if they weren't just recently dropped by a remote player

- Sync player's change to nickname

- Equipment generators
	- Should simply be markers that are used by the modes, depending on the flag
		- later we'll make the testbed conforming
	- is it important now?
		- i guess clientside prediction is more important
		- though we can plan for state

- should rebuy previous also buy magazines bought?
	- perhaps

- Do something so that we don't accidentally discard work in playtesting mode

- Easily spawn loaded weapons or magazines
	- For now, let instantiation load them all by default

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- matchmaking
	- stats persistence
