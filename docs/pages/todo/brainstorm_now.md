---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Fix that camera flying somewhere upon the last kill

- don't give money to spectators
	- just set the money to starting whenever a team is chosen

- item overlaps: choose the item whose overlap point's distance is the smallest to the query center
	- we'll be able to move the center around a circle with just the mouse so it should be cool

- manual deployment only after all of the binaries are built
	- so that the server and clients can go up at the same time

- maybe: properly trim the crosshair raycast to the end of screen and change its color accordingly 
- sort out the crosshair rendering when zoomed out (though we're going to redo the whole thing soon)
- has_size_limit easily just check if the max_size is in sensible range
- sort out those damn max_size/size constexpr traits or get rid of them altogether
	- there is one in explicit pool(const size_type slot_count) {
- move is constant size vector/string to traits
- sort out the max_fps problem

- Check if we don't assume too much about the subject in damage indication system
	- or in draw sentiences hud for that matter
- both should abort early if the subject is not sentient
- be wary of any state that you hold in damage indication system
	- because the ids might change identities or become dead suddenly

- Let only the headshot sound wait for confirmation
- Fix first blood being counted during warmup
- Make the shield a bit lighter for balance

- Item emphasis implementation
	- First pass: shadows
		- Just render all items with pure color shader, with varying intensity (the closer the item to ground, the less shadow)
	- Second pass: actual items
		- Just render the item
	- Third pass: white overlay
		- Just render all items with pure color shader, with varying intensity (the closer the item to ground, the more highlight)

- Emphasize the items laying on the ground
	- Like in hotline miami, they might fly a bit to upper-left direction
	- with a little shadow behind
		- just use existing color overlay shader for that - it will be rendered as the first
	- screw velocity check, just identify the physical filter for it to certainly be the lying item (we don't want to apply it to grenades)
	- a little cyclic shine on the top (might also be done with the overlay shader
	- We could disable emphasis for the empty mags

- We have improve the nomenclature for playing event sounds in bomb defusal code
	- Maybe when we refactor the mode code

- Show a flame icon when we apply damage to a burning corpse
	- Big one on ignition, smaller ones when we help

- fix this..
	- const auto next_explosion_in_ms = rng.randval(0.f, cascade_def.explosion_interval_ms.value);

- Multiple "camera schemes" in settings
	- Fluid (current, hotline miami-like)
	- Rigid (crosscode-like but we'll have to somehow reintroduce the nice feeling of weapon recoil kicking - somewhat artificially)
		- miniscule dead-area like movement will stil be good

- More cool dash effects
	- A transient border-silhouette at the place of initiation, with a sense of direction
		- A pure color highlight?

- Dash animation
	- With all body parts skinned of course
	-Show Only shows when not shooting

- Hide enemy health bars when they are under ferns or other object that's supposed to hide them
- Maybe a little more shake for the explosions of electric triad?

- Killstreaks/combos could add HP
	- To balance one versus multiple situations
	- But people could fight over the frag then

- Blood traces after tearing the body would be cool
	- These need to be entities for proper sync
		- Cool if they'd occlude the neons

- Make dead bodies respond to hits and have some 80 HP as well
	- Then they will explode to bloody pieces
	- If someone has 1 HP they will get torn to pieces on awp shot or even scout/deagle shot
	- tearing apart causes additional loss of money?
		- it should cause high screen shake like a grenade
	- Current damage sounds could be used for damaging the dead bodies
		- and we could use a more snappy sound for normal damage
			- e.g. glass breakage
				- maybe something combined with current analog impact sounds (albeit in way shorter version)

- Hold symlinks in user/ so that they don't get deleted on update
- Position the nade progress circle correctly when reloading

- First reload the one that has less ammo
- Consider not stopping the reloading context on shooting e.g. the gradobicie

- Use ESC to close the rcon please

- We should just show a complete inventory screen on pressing Tilde
	- Escape then goes back to the game

- New inventory system
	- Picking up a weapon should also pick up mags in close vicinity
		- Hover too
	- We can't cycle through same weapon types by repeatedly pressing same button
		- it's because we want to allow akimbo
		- Would make sense with nades though
		- But we have enough numbers to just assign nade types separately

- The fact that we don't want to allow to holster a heavy weapon without a backpack is making it incredibly complicated
	- We might let it switch for both of the weapons
	- or for primary

- Without backpack:
	- Can always have a maximum of two weapons
	- Scenario: heavy gun picked
		- Q just drops it

- Vote pause the game
- Simple commands for chat
	- time

- Release notes
	- release_notes.txt holding last 100 commits
	- Updated on each upload

- Why are indices wrong in shop?
	- Though perhaps it's for the better?

- Show cursor when chat is open
	- And allow for scrolling/selecting the history

- "X killed Y with Z" chat notifications
- Really fix those spectators being kicked
- Dump those logs periodically to files or implement this dump command
	- For masterserver too somehow

- A progress bar for the rocket launcher

- Leave the project selector final touches for later
	- Sorting
	- Filtering
	- Create new arena window
		- We'll only really know what to put in there once we have the editor done

- Makes sense to keep arenas outside of content/official 
	- since we might want to have arena-specific files that won't be intended to be used for custom maps
		- like warmup themes. Not that it will be disallowed but it will encourage own creations

- Well.. it's a pain but we'll still somehow need to order entities within an order
	- Unless we spam with these flavours like crazy
	- Well what if we want to have the same decor in both background and foreground? Makes little sense to make another prefab for this, better just auto-generate two different flavours
	- we can always put the sorting order in a component instead of invariant
		- since, at this rate, it's pretty much something like a transform
	- We could screw the sorting order and just sort by ids
		- Fast, though is this what we truly want?
		- Not really. Consider different entity types

- New entity type: tile layer?
	- Shouldn't really be rocket-sciency to support animations later
	- Do we mix tile sizes? Preferably not...

- We'll move insects and other stuff to official only once we have a general outlook at the editor
	- No point in doing this now

- Area marker variation (?)

- All obstacles should be on a well-defined layer
	- like in jj2
	- Well what about detached heads?
- The point is what the user can see
	- Ground and Foreground will not be physical
- What about mixing physical and non-physical tiles on the same tile layer?

- I think we need to let go of the concept that "render_layer" will directly correspond to the stuff in "Separators"

- Do we really want to expose "wall lighting" option to ground layer even when it will make little sense?
	- Perhaps make it only appear for the wall layers and all wall flavours will have it on by default

- Notice that we will have to make likewise "separators" for callout markers, lights and whatnot
	- So shouldn't they be render layers too?
	- We could call it a special layer
	- All special layers will be a heterogenous tuple of arrays of possible id type vectors
	- I'd divide it into tabs
		- Foreground
		- Background
		- Special
	- Though it would be enough to just have a single sub-layer of lights and of markers
		- instead of whole separators

- Maybe instead of "glass obstacles" and "solid obstacles" have just solid obstacles and a bool whether we want to apply additional wall light
	- similarly with foreground?
	- but the glows would anyways be rendered on top so it'd be counterintuitive
		- similarly with neon erasers
		- we can have though instead of SOLID_OBSTACLES_OCCLUDING_NEONS just a single solid obstacle layer and just a flag if to occlude floor neons
			- since it will anyways be shown always above the ground layer 

- Sentience -> Character please

- Fix version numbering on MacOS
	- It's probably because of the shallow clone

- Left-handed option

- Fix those mag refills please

- Fixing wallbangs and wall teleportation
	- A short raycast sensor in front of the player
		- Detect all convex shapes of walls in front of him
		- Just take all vertices of all detected convexes
		- And create a single trapezoid body
	- Don't do raycast
		- simply query the  right before the physics step (after applying the crosshair's direction to player rotation)

- Leaderboards shown in the main menu
- "Host a server" button right in the arena builder

- Look for some filesystem implementation for imgui
	- We'll also replace the system viewers? At least some of them?
		- Only reveal in explorer will be useful at this point

- A setup needs to be able to launch another setup easily
	- A callback with a launcher passed?

- Rock the vote for modes, schemes and difficulties

- We can pay someone to properly re-map all of the official maps in the new editor.
	- The ultimate test.

- Wtf with those mags appearing at the center of the map?
- Also you can still sometimes get a magazine in hand if you buy HPSR with little inventory space
- And you still sometimes need to reload weapons after the round is restarted

- Player can see the opposing team's inventory post-mortem

- The aborted sounds end abruplty if we have less than 1.0 master/sfx gain in settings

- (Update script) Sync config.lua against the user folder in the home dir, not in hypersomnia dir
	- otherwise we have to call vim_build AND vim_run later

- Post-map-format-fixes
	- match begins in seconds for de_labs2 in bomb_defusal.cpp
	- ultimate wrath of the aeons FX fields are used for electric triad

- Energy consumption fix: sleep the remaining frame time

- Look for imgui demo's Console for a nice copyable log text field
	- separator under button

- Fix flashbang sound volume

- Camera should also ease towards new positions instead of resetting completely

- Fix crash when the read value is a value and lua readwriter expects a table
	- and the other way too
	- just handle an exception probably

- Turn off logs of masterserver and browser later

- Implement sending the current version to the server

- Remember to not send goodbye for when the servers are automatically updating

- Apparently, clipboard prevents connection to any server...
	- Because it can't bind the socket. lol

- The masterserver should dump entire state before restarting for update
	- To give servers a chance, set time of last heartbeat to current time after starting up

- It's not really a problem if the server list is recalculated in the background.
	- The bandwidth used is minimal, especially since we've imposed a limit on the number of packets there.
	- So let the advancer just go all the time.

- later determine why heartbeats mismatch every time

- We should give up on opening hosts that don't respond

- Just pass a lambda for ingame menu which buttons are available
	- Later we'll properly hide them but for now they'll just be inactive

- IPv6 fixes
	- ip-agnostic find_underlying_socket

- advance only advances pings and nats already requested
	- but it is only the imgui-performing function that requests the pings and nats in the first place
		- and only if it detects that there is a need for them and a sufficient interval has passed

- For now, don't save in custom config after connecting from the list

- If someone connects at the last slot available, and there is no master RCON on the server, kick them if the rcon doesn't match
	- So that we always have a slot registered for RCON

- Detecting servers as official ones given just masterserver list 
	- The client anyway has to resolve official server ip addresses for pinging
	- match ips

- Find best official server?

- Resolve masterserver hostname once a minute or two, asynchronously?
	- mutex onto result netcode_address_t

- just take sockname from the existing connection with masterserver over http
	- when you downloaded the server list

- don't change map structure until we finally make demo player improvements
	- which we need for demo replay
	- or make a branch for demo replay improvements and rebase unto the original commit later

- I think let's just first do nat punch over external ips
	- let's search for how people do those internal ips on the internet

- check how getifaddrs works on the linux server to see if it properly returns external ip
	- Well, it doesnt...

- Later only respond to ping if the request packet is 1000 bytes or so
	- to avoid ddos

- Don't add the option to make MS "assist" your client in connecting to direct ip
	- It will be anyway easier to just search for "Kumpel's Server" in the global server list than to paste an ip
	- Servers most recently hosted will be checked for latency first

- Add RCON to server and client in-game menu

- Server name to server vars
	- server sends it to masterserver next time

- Info about official servers
	- We still want official servers to send periodic info to masterserver 
	- because they will be in the list in the main menu too
	- Website just downloads list of masterservers
	
- Security concerns when rendering on site
	- We need to consider that arbitrary string might be found inside server's name, map name or player username

- Sending server stats

- Website server status
	- Game servers have a built-in http server?

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
	- send some heartbeat on mouse movement
	- or just not kick at all
	- actually instead of kicking, just move players to spectator for an indefinite period of time

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
