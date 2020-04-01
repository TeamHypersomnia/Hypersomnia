---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Don't use client/server's underlying socket for other stuff!!!
	- It might break netcode logic because netcode packets will be hijacked by the other logic

- Perhaps it's the time to upload the masterserver
	- don't forget about the dedicated server
	- test masterserver locally beforehand

- Later fix number of probed ports to like 3 maybe

- In a single app run, keep a dictionary of opened servers and ping them regularly
	- could even perhaps ping them from the browser window
	- so, the browser window could fulfill that role entirely
	- for that, the browser window will have to use the auxiliary socket too, but why not

- I guess client connecting screen will just show a separate window with autoresizing, same as in server start gui
	- Can only show the last log line in the small status window
	- and full log in details?
	- nah, maybe let's just show everything in the small window somehow and forget the separate window (?)

- Even though for reconnects, we can bind a new source port for the client,
	- with server it's not so possible
	- so if server has symmetric nat we'll still have to somehow remember the port it has opened for us
	- masterserver could somehow keep it but I have no idea how
	- app of course could keep it
	- if we restart the client though, we can get a new port
		- and then the server will map a new port anyway
		- so ironically it's worse if the server is address sensitive and not port sensitive

- Don't query the stun servers/port probes again if we want to reconnect right away
	- We might want to reuse the ports for reconnection

- If one is at most conic and the other symmetric, let the symmetric initiate connection 
	- Even if it's the server. The client will just "request_nat_hole"
	- this even handles client being a public internet host and the server being symmetric nat
	- if both are symmetric, it doesn't matter which one resolves new port first

- Look for imgui demo's Console for a nice copyable log text field
	- separator under button

- Symmetric nat punch (finally)
	- Punching servers one by one
	- While sending a punch request to masterserver, include our own last predicted port
		- If we're punching for the first time, we can get it from the my network details window state
	- Let's first acquire all data
	- Server can too send its nat type to the masterserver
	- links:
		- https://slideplayer.com/slide/3159695/

- Fix flashbang sound volume

- Investigate the mysterious crash
	- Happened when we finished drag and dropping on hotbar 
	- All buttons on hotbar were assigned
	- The item we dragged was hidden in backpack and *possibly* unassigned to hotbar yet

- Camera should also ease towards new positions instead of resetting completely

- Test results:
	- Two totally different addresses gave us proximate ports, so there's a chance for us
		- 35.205.19.61:43214 -> 31.182.205.239:28794
		- 35.205.19.61:43215 -> 31.182.205.239:28795
		- 104.199.81.130:8430 -> 31.182.205.239:28778
		- 104.199.81.130:8415 -> 31.182.205.239:28776
		- later 31.182.205.239:28816 and 31.182.205.239:28817 for the official server

- after testing with another nat masterserver, looks like ports are assigned similarly here
	- just incremented
	- we'll see it on another day
	- test with a completely different port at the server
	- test on Windows server

- Symmetric nat punch
	- We can easily determine server's port allocation rule
		- if both are address sensitive 
		- if both are port sensitive we're fugged
		- if at least one is non-port sensitive it should be relatively easy to establish connection with bruteforce
	- remember to punch nats one after another
	- Also client's port allocation rule
	- And with just the masterserver. It will just open another port that will respond
	- https://sketchboard.me/zBYca50ZCLaL#/

- Fix crash when the read value is a value and lua readwriter expects a table
	- and the other way too
	- just handle an exception probably

- Let's just do a simple quadratic approach for now
	- You can't send just a single packet from the target server, beacuse that port is reserved for my communication with the master server
		- But at the very least it can be linear on the server-side
		- You just send at like 100 sequential destination ports and call it a day
		- Then you brute force bind e.g. 100 sockets on client side and for each of them, open all possible source ports at which any from the 100 server packets can arrive

- Symmetric nat punch
	- https://github.com/ph4r05/NATPaper
	- https://github.com/P2PSP/core/blob/master/doc/NTS/NAT_traversal.md
	- https://doc-kurento.readthedocs.io/en/6.11.0/knowledge/nat.html

- Turn off logs of masterserver and browser later

- Implement sending the current version to the server

- If both masterserver and server are on the same machine then nat punch request might not arrive on the lane masterserver->server
	- Which is why we're seeing official server offline when testing without pre-emptive ping
	- To test, temporarily just send a nat punch request to localhost on masterserver
	- For testing, just request to punch the internal server's address

- Remember to not send goodbye for when the servers are automatically updating

- bomb_defusal -> bomb_defusal

- Apparently, clipboard prevents connection to any server...
	- Because it can't bind the socket

- We'll detect whether the server is internal at the server browser stage.
	- Because we'll ping both addresses.

- The masterserver should dump entire state before restarting for update
	- To give servers a chance, set time of last heartbeat to current time after starting up

- It's not really a problem if the server list is recalculated in the background.
	- The bandwidth used is minimal, especially since we've imposed a limit on the number of packets there.
	- So let the advancer just go all the time.

- later determine why heartbeats mismatch every time

- We should give up on opening hosts that don't respond

- Just pass a lambda for ingame menu which buttons are available
	- Later we'll properly hide them but for now they'll just be inactive

- Only allow browse servers

- IPv6 fixes
	- ip-agnostic find_underlying_socket

- Blindly request whenever we connect anywhere, in client_setup
	- But do this asynchronously to the connection attempt
	- won't hurt to send the nat open request once again upon connect from the list
	- Main menu can pass resolved netcode address of masterserver
	- If starting client setup from scratch, we have to resolve it
	- So, two overloads?
		- Just one but have on optional resolved
	- Well, we don't have the resolved masterserver address because it's from httplib

- Browse window logistics
	- Main menu holds a server list socket
	- In-game, the yojimbo's socket is used
	- Close upon entering but when launching another setup, ask to re-open when going back to menu
	- Re-calculate server details for showing in-game
		- Though we won't show internal ip address but it's okay

- Problem: we can't browse servers in-game unless we use the socket provided by yojimbo
	- What if we overwrite the socket in yojimbo?
		- nah, we have no way of tampering the connection process
	- let browse gui hold a pointer to the used socket
	- let main menu hold such socket
		- hold it in main menu even if we don't allow to watch the server list from the game
			- because we anyway have to destroy it before connecting 
	- later the socket will be destroyed along with the main menu

- We have no choice but to simply pass around the preferred port as a value and hope for the best

- advance only advances pings and nats already requested
	- but it is only the imgui-performing function that requests the pings and nats in the first place
		- and only if it detects that there is a need for them and a sufficient interval has passed

- For now, don't save in custom config after connecting from the list

- Client-side, on receiving server list:
	- 

- Let the client manage the nat punching for all the servers on its own
	- Server would be too vulnerable if it would send a ping after getting just a single packet
	- At least it will depend on the upload strength of the client
	- Masterserver doesn't have to worry about current punching state, just relays reverse requests

- Somehow limit the amount of possible nat requests existent on the server

- Let's just do ping synchronously for now, we'd have to have like thousands of servers for async to make a difference
	- synchronously receive packets in perform imgui

- Watch out because perform logic might suddenly stop being run since the user can just close the window

- It's safe to send at least 64 ping packets per perform tick
	- Since it's tied to the frame rate and a hypothetical server must be able to handle that

- Already when the server list is requested, masterserver should request a reverse ping from all servers 
	- Then just automatically re-ping all servers once every 20 seconds

- By default show most recent servers on top
	- nah, actually the closest

- Later, upon choosing a server list entry, re-request the masterserver to aid in the connection, just in case

- Add more commonly changed parameters to host a server menu
	- Name and arena

- Will TCP be available after nat punch?
	- because we'll somehow need to transmit the map data...

- If someone connects at the last slot available, and there is no master RCON on the server, kick them if the rcon doesn't match
	- So that we always have a slot registered for RCON

- Detecting servers as official ones given just masterserver list 
	- The client anyway has to resolve official server ip addresses for pinging
	- match ips

- Consider a tab for "official" and "community" in browse servers
	- Actually maybe no because it'd be nice to have it condensed and compare best pings quickly

- "Favorites" tab

- Remember the official server to which we have connected

- Determining best official server
	- default: EU
	- Once every second ping all official servers
	- change_with_save to the first one that responds
	- is later chosen as the default for Connect to official universe
	- show ping live in that window next to the server name

- Quick play
	- From servers that have any players, find the one with best latency

- Find best official server?

- Resolve masterserver hostname once a minute or two, asynchronously?
	- mutex onto result netcode_address_t

- just take sockname from the existing connection with masterserver over http
	- when you downloaded the server list

- Procedure for the game server
	- Once every 5 seconds, send statistics line to the masterserver
		- Contains external/internal address:port info
		- num of players and all these stats
	- Masterserver smoothly registers the server every time it encounters this
	- To send stats, use the socket already bound in yojimbo server, somehow extract it
	- Responding to nat open request
		- same as ping just different enum and has a target address attached to it
		- send back confirmation to source packet ip if different
	- Responding to ping requests
		- A potential game client will ping this server
		- We only need to send a ping back
	- It looks like we'll never send stats on-demand; only periodically - completely on our terms
	- so netcode only needs a ping packet
	- has to have a constant connection with the masterserver in case that it sends information about connection request

- Procedure for the masterserver
	- Open port 8413 at all times
	- Receive:
		- list requests
			- HTTP get
			- has to open NAT of all the servers, for the requester to know ping
				- only open nat for servers behind a nat
				- send nat open request every second, marking those which managed to receive the message
			- at this scale we don't have to worry about it, just set a timeout for refreshes, like 10 secs
		- detailed server info only on demand through http
		- connection requests
			- to external ip:port
			- yojimbo provided socket
		- heartbeats from servers
			- yojimbo provided socket
			- If no messages came for a minute, delete the server from the list
				- Mapping by the external ip
	- Respond with:
		- list contents
			- to clients
			- to website
			- http
		- server details
			- http
		- connection requests
			- but only to the target server

- Procedure for a potential client
	- Downloading list of servers
		- We'll use HTTP because the masterserver will anyway have a http server for other uses
- Use our test windows server to check how external ip resolution works on windows
	- though it might be screwed up too

- log detected internal ip on startup

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


- Data format


- Map version considerations
	- Communiy map migration
		- copy user folder to OLD_HYPERSOMNIA, in case there are some important maps too
		- Solution for map conversions: just keep binary versions in caches?

	- Check official map version upon connection
		- if mismatch
			- custom? re-download
			- official? disconnect

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
