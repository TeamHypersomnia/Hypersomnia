---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Tutorial levels
    - Planting/defusing bomb?
    - Spells but maybe just preliminary
        - Press haste, kill enemies with the triad etc.
    - Surfing
    - that's all, final portal goes to the shooting range
        - or maybe two portals for that?
- Done
    - Grenades
    - Walking
    - Ricochets
    - Knives: 
        - Kill with primary/secondary, could be just one level
        - throwing especially with scroll down/up
    - Akimbo
    - First weapon
        - Choosing with numbers (have to for akimbo)
        - Holster/previous weapon
            - Maybe the green areas could teleport when they detect step completion to mash several steps into a single level
        - Toggle laser, show which surface is shoot through

- Fix miniatures not downloading!!! Both externally and through UDP
    - Don't make a "metaresource", this will overcomplicate things.
        - Just force it to become a normal resource but it is always referenced (if it's backed, always with non-default properties, if it's unbacked, always default)

    - disregard:
        - Easiest will be to just always add the miniature to the external resources and never show it
        - Just have a separate external resource field in editor project so that it doesn't appear as a valid resource in editor
            - but gets written to json and thus requested when downloading nevertheless

- Simplify stamina
    - Remove the white edge denoting limit
    - Dash should just take stamina on credit like in souls
    - We can make the dash a little more expensive to compensate so that we can still do 4 dashes on a full bar
        - This game is too fast anyway so we wouldn't want more dashes here

- Settings: split Gameplay tab into HUD and Rendering (or idk name it somehow else)
- Editor: Fix lighting inconsistency between with and without FOV
    - Seen on shooting range with wood material
    - we'll use vent for the tutorial

- recheck what happens if missing arena is chosen, afaik it chose de_jedrula still and reported hash of 0

- Editor: Grouping.
    - Ctrl+G groupizes selection.
        - Duplicating a group instantiates it.
        - Can group several entities with a ctrl+g and immediately create instances this way.
    - Group instance parameters.
        - Position
        - Rotation
        - Colorize (optional)
        - There's not much
        - These are the only things visible in the inspector when you inspect a group instance.
    - (Hard) Editing/customizing grouped subnodes.
        - Proposal: have a "Groups" dock appear splitting below Layers.
            - Only groups are visible, can expand them like layers.
            - The subnodes then are editable.
            - Would be best to have a full-fledged screen on black background for editing a group.
                - because then grid will work properly etc
            - automatically shown when you select a group subnode.
                - goes back when you select something in Layers
                    - or just when Layers window activates
            - And drag-dropping the top-level group instantiates it like a resource.
            - Groups in this window are sorted by dependencies.
    - (Hard) Serialization.
        - Would probably need to serialize them before nodes.
        - Even though there are nodes inside too, but they are more "virtual"


- Fix spawns in gungame mode - they're currently shuffled per-round

- Alright we really need to sort out how custom maps are stored on the server
    - for now just
        - symlink user/downloads to ~/community_arenas per each update
        - page with all downloads etc. should point to ~/community_arenas and say "Community Arenas
    - ultimately
        - 1) Separate official/custom maps on hypersomnia.xyz
        - 2) Keep official ones immutably in content/arenas (done)
        - 3) Keep custom ones in .config/user/downloads/arenas and point the php page to it (works with .tar.gz too just have to symlink each time)
        - For custom servers it sorta works out of the box since they don't serve the php page

- Server might choose a map whose new version is available
    - For now: We'll have handled most cases if we auto-update the map we host. Or prompt to update it.
        - Why?
            - The dedicated servers will probably restart periodically
        - People will most likely choose the catalogue to choose the map to host either way
            - but won't hurt to prompt

    - Might happen when hosting from the main menu..
    - ..or during gameplay
    - in both cases we can just have an std::future that switches to the new map once it's completed
        - and simply prevent choose_arena from choosing this map
        - in which case, if it's hosted from the main menu, it'd go to the test scene
    - However we need to check if a new version is available and that takes a bit
        - So we'd like to always switch to the new map preemptively, maybe at least when hosting
        - and then run the updater in the background, the files will have been loaded anyway
        - while we check for the new version, prevent direct udp download?
        - in this case we'd like to avoid checking for hash when client downloads externally, because the external provider will be the source of truth
- Syncing maps (even manually through RCON) might break existing UDP download sessions
    - We either:
        - Load all files to memory per hash but it will grow dangerously
        - Prevent syncing any files until no udp download sessions are live
            - Only problematic until we implement kicking/banning


- maybe migrate server to appimage after all
    - but we'll still have a problem with syncing the new config force lua from the repo
        - maybe we can sync only this file and have our own config.lua
    - just prepare one with -g for the server
    - will still require less space than tar.gz + sfx
    - just find out how to not strip debug symbols from the appimage 

- masterserver can easily be controlled with https+api key

- keep being spectator when half ends

- a notice on /servers to not launch with screen or something else to be able to autoupdate

- Updating all the servers automatically
    - MS could send packets with signatures of the new version 
        - Then the servers will know it's legit, even if it's spoofed it literally only helps
        - We don't even have to download the entire archive then. We'll just have one more thing to sign, the new version message.

- Map catalogue
    - Note external arena provider and ingame catalogue source are conceptually two different links
        - even though they'll coincide for the official server
        - BUT After all let's make the catalogue read from the server vars.
            - Why? Because if the end-user chooses a different provider they'll expect that hosting a server will take the same link as provider too.

    - Problems to solve
        - What if we update maps while the servers already have them loaded?
            - The existing clients still need a way to download the correct ones
            - The custom servers will likely have tcp/ip file downloads enabled so it should still work as a backstop
            - Whereas we can take care to restart the official server when we update maps
        - The server has to know whether the arena it hosts is actually available to download or it will constantly send wrong links
            - But the link it sends is the same anyway; why not just send the host in the settings and let the client figure it out on their own?
                - let the client figure out whether the map is on the external provider and just send requests if its not

            - Why not just query the json file at the host every time the map is hosted/chosen by the server?
                - If it differs, or can't reach the host, only allow direct transfers
                - Can even query every time someone wants to download
                    - the client waits anyway for a file payload so they can wait until we query the host too no prob
                    - anyways if it's hosted from user/projects than we always disallow anyway, we only check with user/downloads

            - btw we'll just send the download link once which will be the arena root and the client will download the rest on its own

            - Plus the dedicated servers with public ip can actually make server's file coincide with what is served over https 

            - How about we determine it by whether it's in user/projects or in downloaded arenas?
                - though downloaded arenas might be from somewhere else too
            - if user/projects - always direct
            - 
    - First let's replace udp downloads with direct http downloads
        - Idea: server could provide a link instead of an actual file
        - This is tempting because it's the server's responsibility then to provide up to date files and we can preserve the whole flow
    - Generally, the client asks for hash - it gets either the block message (or tcp/ip connection request) or the external link message
- Maybe it's easier to just skip the bullshit with optional links and just start tcp ip connection
    - Most of the time arena files will be hosted on the same server
        - Well they have to have these files anyway to run the simulation
    - yea i think it'll be better for now
- Unless we autoupload any map to the host
    - Yeah I think it's bad to assume tcp will also work after punching
- In any case, if we're not doing tcp/ip, we need the file_download_link payload regardless if we're doing uploads or some relay


- post external fixing
    - nah it's something with hashes actually, probably autosaves at work again, or clrfs, see the downloaded logs

    - maybe increase http timeout to 3? if there were problems with externals we'd see it on our own
    - send stat keepalives more often because they get stuck or just not stop sending packets at all
    
- We could also use UDT later, looks legit
    - just send the list of all hashes once and send it in a single chunk?
    - per file is okay too I guess

- guess for now we could leave a conservative number like 1mb and focus on ingame map catalogue

- we could just send big-ass packets and hope for the best but with just 1% packet loss we'd lose 10% packets at 600 kb/s (since then i'd need to send 10 packets), that becomes way worse if the loss is 2-3%
    - so literally any block based approach that i come up with will be vastly better

- well the udp approach is certainly more fun and decentralized
    - and we can easily read the files asynchronously
        - note that since the messages are by definition unreliable the clients themselves will repeat the requests until the files are loaded lol
    - just have to send the project file size and then ask for the list of all file sizes in a block

- wait.. can't we just setup a tcp/ip relay with the masterserver?
    - We need it anyway to establish a connection in the first place

    
- Such large packet buffers could really fuck up our performance
    - and these udp transfers might still fail
    - but let's at least test it

- Http downloads from external arena provider.
    - Problem: a server might easily end up with out-of-date maps on disk.
        - Either someone might start a dedicated server without updating it (though the normal game client will autoupdate 'tracked' maps on start)
            - Or a running server might end up having out-of-date maps if new ones are uploaded to some public provider.
    - The server can check for a specific map update *before* launching the map.
        - The previous map can simply be on indefinitely
        - Could even be precached during match summary

- add player lists to server browser

- map statistics from server_list_json
    - counted only when >= 2 players are fighting

- show tips with names in test scene setup

- verify those map file hashes either during upload stage or hosting stage

- settings ui overhaul
    - put some controls switches under Advanced
    - logarithmic scale for sound?
        - and also make default master less than 1

- How do we go from here?
    - To not have to repeatedly setup some common ruleset data too (like announcer sounds), we'd have to have not a variant, but an actual tuple of all sub-modes.
    - This means we'd pretty much have a single ruleset.
        - But editor wise we'd have several.
        - and they'd allow custom announcer sounds? That would be silly too I guess
        - although technically not a worry for now until it's doable
        - but we need to think if we shouldn't make some common "arena" rules common for all of gungame, bomb defusal, tdm etc

- We run into the same problem in editor specification
    - Things like announcer etc will repeat
        - however I'd argue these things won't really be editable
            - at most a map can e.g. override some sound files by path


- server should send the list of maps it has to the rcon

- flavour cache id should only be built under build_debugger 

- Accessibility fixes before Steam
    - Tutorial
        - Text Areas
            - Will have to support keywords in descriptions for actions like [PULL_OUT_BOMB]
        - Akimbo
            - For things like this we could have auto-regenerating entities
                - and both must be 'dead' at the same time to pass
                - but not at 0; at some < 0 value as remember the bullets won't hit continuously
    - (press "Y" to reply)
    - Per-setup defined Status bar in in-game menu
        - e.g. Editor - de_cyberaqua
            - or multiline when playing on servers like Playing on PEPSIK server\nde_cyberaqua\nGun-game
        - In some cases equivalent to window title

- win_sounds -> win_announcers

- Gungame
    - Now that the game supports >1 mode
        - Choosing the mode from Host a server
            - the default will have to change along with each choice map choice and it has to say e.g. "Gun game (default)"
        - Choosing the mode from F8
    - Spawn protection
        - Reuse teleport alpha logic
        - Disable collision resolution in missile surface
        - Just set a timer in sentience
    - Disable dropping
        - Except for knives
            - For this let's make all dropped items disappear after 10 secs, like in test mode
        - We don't want other weapons to be dropped because people will just drop stronger weapons to the team leader
            - They'll still be able to pick others' knives though, but maybe it's not that bad
    - Basic equipment
        - So that we can specify basic knife/armor/backpacks too

- Portals should take precedence in the layer order
    - Why doesn't it work out of the box? It should because we're iterating them linearly

- Optional: set_and_add_velocity mode that redirects old velocity AND adds another impulse
    - Or maybe a flag - redirect velocity
        - Note however it doesn't make sense if e.g. the exit direction is set to entering velocity
            - well in this case the redirect will just have no effect
                - so i guess it only makes sense when portal exit direction = portal direction?
    - "redirect_and_set_velocity"

- Fix a performance problem with dashed lines, on a per-line basis

- We should totally make a convenience resource for force fields
    - and it will be a box by default with some nice particles like in portal, but rectangular
    - and maybe can add a sprite from the get go although that won't be necessary for now

- chase velocity flag for exit portal particles
    - so that they can be seen on an extremely fast moving character

- ensure exit sounds are played in place and only the listener is set to the traveller subject
    - bystanders would otherwise hear insane doppler effect
        - though on the other hand if you and your friend are flying together you'll hear his exit sound with insane doppler but idc

- corpses i think dont teleport
    - they'll automatically react if Characters are ticked
        - For surf ski shouldn't be a problem because we won't respawn the player in place like in quick_test
        - it will always respawn the player at entry so they can get ready to start again

- either remove bursts from the enum combos or automatically make them streams somehow

- "Override map setting (not recommended)" for FOV
    - And an optional FOV setting

- portal finishing touches
    - fix decrease opacity math
    - good particle effects
    - setup good defaults w/sounds etc

    - btw watch out if components::portal doesnt have any state that needs synchronization actually
        - however sound cooldowns are only view related so they don't need to be

    - done/disregarded
        - thrown melees get teleported faster?
            - Was likely due to more amount of contacts for convex bodies.
                - solved with m_last_teleport_progress_timestamp
                    - this also let us handle characters holding weapons

            - same with dropped items though
            - maybe they get processed twice due to attachments?
            - melee has no attachment tho

        - several simple "color presets" could let us avoid the need to tweak a million color controls
            - json wise we'd like to have just color_preset: CYAN/ORANGE/GREEN etc
                - and only writeout the customized color fields
                - however this will complicate our default setup flow as defaults will now depend on values
                - but is there any other way?
                - we'd like to let people just specify the color
                - what if they just override the color settings?

        - maybe allow box
        - "require zero inertia"
        - separate "portal inertia" in movement that doesn't remove the ability to move
            - and hard limit to like 3 seconds it so that it doesn't accumulate when you enter a loop and suddenly exit it
        - setup default effect ids
        - set/add character inertia (const/linear)
        - Make these exit portal parameters:
            - Exit position
                - Always center
                - As entered
                - At boundary
            - Exit direction 
                - Portal direction
                - As entered
        - setup default effects
        - fix alpha so that a=0 shows only when we actually teleport
        - and add some easy descriptions for impulse/add velocity etc since it's complex!
        - remove FORCE so that people don't set it wrongly there and get confused unnecessarily 
            - think we should just have a separate editor_impulse_type without FORCE whatsoever
            - the force field will *not* have all these options, they make no sense there
                - Only the force amount and whether to scale with mass (Proportional to mass)
                - Should be mass-invariant by default so not proportional, like everything moves there at the same speed
                - Characters will need some continuous inertia probably (maybe by setting it with std::max between current and target?)
                    - certainly makes sense for characters to be more inert in the force field, will make for a nice surf experience
        - post a pure-color highlight on entry and exit

        - ignore portal once teleported through it
            - reuse dropped cooldown

        - calc_filters from 
        - debug json serialize in parent
        - check how cache creates fixtures for it despite lack of sprite invariant
        - convenience flag: trampoline_like that automatically sets delays to 0 and sets entry offset preservation to true
            - sets portal target to itself
        - cooldowns for sounds
            - because shotgun pellets will spam them massively
            - or just make a special case for shotgun pellets tbh
                - like only the first of the 'series' spawns a sound
                - solved: just ignore for shells/bullets/remnants altogether for now

        - snap interpolation
        - stop begin sound effect when quitting portal?
            - not sure if this makes sense tbh
                - begin sound should be short enough
                - and we don't want to adjust logical length to sound length because we want easy customization of delays

            - this will allow us to set one sound for both start and culmination  
            - "continuous entering" sound effect can simply be implemented by adding another ambient sound with a small range
        - We will reuse portals as force fields
            - The logic would be nearly identical so there's no point introducing another entity/node type
            - Force field will even be a useful effect for the portal as well
                - We'll give one by default that pulls towards the center
                - By giving a minus someone can set it to pull outwards
                - And also torque
                - And just a flag whether it should pull proportional to mass
            - Let force fields increase portal inertia
                - slowly with a maximum, two parameters per each force field


- FFA planning
    - Lying weapons are unlimited
        - They replace the ones currently in inventory
        - It is the mode that determines lying weapon behavior
        - Although if a map supports both defusal and ffa we'd like to remove the lying items on bomb defusal mode
    - As a first iteration we can just set default equipment for resistance and metropolis and only let players choose there
        - yeah lying "armory" items will be next mvp

- we really need to fix that angular/radian/degree nomenclature for velocities

- We need to rename all "specific_" to "typed_"

- remove update_official_content from debugger setup

- Setting "hurt" to materials is tempting but this should really be as a "on_collision" event
    - Same with spawning explosions etc

- We might need to have global settings for collision pairs after all
    - The default ones should always let the other play

- We should handle "scripting" before "grouping"
    - Internal references to nodes need to be properly resolved
        - This is even the case with simple cloning of say teleport + teleport target

- When thinking of events, it's the most important to plan for the json format
    - Note that instead of setting "conditions" for "on enter", we can just conditionally spawn entities that have these properties set

- Watch out when adding reference counts to materials and other specific resources
    - Them being referenced will trigger a writeout of external resources
    - We need to separately rescan them 
    - rescan_resources_to_track should really be rescan_resources_to_track
        - should it return a bool info PER EVERY POOL?
            - Actually not, just return for externals but properly detect if it's these

- foostep speed could be fixed within the same parameter tbh

- Deploy last builds should not delete uploaded maps
    - Why not keep it in the server's user/downloaded?
        - well we want officials too to be listed on the page
    - test it

- Simple event system
    - First usecase: Teleports and Playing sounds
    - Best to be able to attach it to any entity
        - No additional complexity cosmos-side as we'll have a map from entity ids to events
            - since events will be scarce
    - Most event data should not be transmitted through the network
        - As it doesn't change generally
        - So we might even want to have it in solvable
    - However even simple playing sound will want to have some state
        - E.g. whether the sound is already playing, to not spawn a new one
    - Separate marker type for event areas
        - We thought about it but it's pointless
        - The only advantage is we could make them sensors and they could respond to onenter/onquit

- Teleportation Target node specification
    - We might want to specify velocity after teleport
        - Several teleporters might point to the same target
            - Would then be nice to only specify the velocity once
        - Teleport target point marker then?
        - Note it would be silly to make "any node" a potential teleport target because it would have to be rotated in a direction too
            - So a point marker will be best here
            - And if it's a teleport target it will have an additional "magnitude" property
    

- Fix callouts rendering

- Investigate why is changing maps moving to spectator
    - same thing when entering playtesting
    - probably something with faction remapping
    - is it connected to the vars != nullptr crash?

- RCON should show maps on the server, not the local ones

- On the official server, would be best to have a separate folder for custom maps
    - because we have to reupload every time on deploy otherwise 
    - and a config switch for that
    - or just let the server update with the self updater

- tooltips for physical props etc
    - and also for generate miniature button

- the whole bullshit with perform_transfer would be a non-issue if our inventory system was normal

- maybe don't use sleep on dedicated server as it might not be precise

- warmup theme doesn't restart if it stopped and someone joins and game commences
    - Shouldn't we resample sounds when we reset round?
        - Not really, we don't want sounds to end abruptly

- Fix organism area order
    - Only the topmost one should be considered

- impose that 10mb limit in editor too

- point/area markers -> spots/zones

- remember to uncomment looking in editor files once we're done
    - and dont commit webhookd suppression 

- fix broken chat when client downloads files, although not that important

- log should call to hex format automatically

- Net solvable is too large for such a simple map, check other maps
    - The official content populator was reserving entities... 3000 for each type. Fixed.


- Note client will be disconnected if arena contains files that do not exist

- Lain ambience could be good for some maps lol

- Remember about proper reinference both on the server and the client whenever complete solvable is sent
    - The client reinfers anyway whenever a snapshot arrives

- Won't chat be broken if we don't send solvables?
    - Since displaying it depends on the mode state
        - Not necessarily, I think chat messages are fully formatted before being sent

- Could the server's first initial_state_payload be misconstrued as the post-download resynced arena?
    - Solution: simply send files in the same channel as the solvables
        - Bonus points: less memory wasted
        - Another bonus points: state snapshots can use more MB
        
    - That would be bad
    - The initial one could very well arrive well after the arena is downloaded

- is_gameplay_on returning false should take care of arena handle usage
    - What is being rendered while we download though?
        - Especially if we start once we were already in the game
    - get_viewed_character_id will return dead entity when is_gameplay_on is false
        - so it should be fine, the old arena proabably will never be referenced in viewing code

- Shouldn't we handle networked_server_step_entropy?
    - Point is to clear local pending entropies
    - But if we resync after finishing download we should begin with no entropies at all anyway
        - Watch out for receiver.predicted_entropies, this might get filled
        - Shouldn't we anyway only send entropies if we are on a valid arena? This will fix this problem
            - Unless there are already some receiver.predicted_entropies but suddenly the map is changed and we didn't clear it
            - Note the entire resolution of accepted entropies and clearing them is resolved under if (in_game) {
                - in_game will be false when we download
                - Then I believe we should clear the pending entropies when we start the download
                    - But the server should also clear the incoming ones in its client state
                    

- Careful about avatars. See if new players are synchronized correctly while we download.
    - Technically any new avatar should go to untimely payloads as there will not be any arena
        - And it should properly be handled once the resync arrives with new arena state where handle_untimely_payloads will find the relevant session ids 

- For resyncing, let's revert the client back to PENDING_WELCOME.
    - Not sure anymore.
        - This might be less predictable once connection initialization is more complex
            - Since we force it to be repeated entirely
        - Maybe just explicitly resync the entire solvable
            - Since we know it's the only thing that gets paused during arena files transmission
        - And have an explicit clause for when the client is downloading
        - This is tempting because client's gui would react out of the box
    - The real q is what does the client do when it knows its state is IN_GAME
        - E.g. it sends the avatar
        - For the initial connection, it might even be the case that we won't enter IN_GAME until after we have downloaded the arena.
            - The solvable might arrive first before initial arena payload arrives
                - And we set the state to in_game only once initial arena payload arrives
            - So player's avatar wouldn't arrive until after we've downloaded the map
                - Not a bad thing; they will be in spectators anyway and this way we even prioritize bandwidth
        - So the q is if the game will behave correctly when we can stay in downloading state both during IN_GAME state as well as RECEIVING_INITIAL_SNAPSHOT
            - Cannot be an earlier state since the first server_public_vars triggers RECEIVING_INITIAL_SNAPSHOT
                - it will stay in that mode until we finish download
                    - that is because IN_GAME is triggered only with the first initial arena state payload

    - This way we can revert the state easily both the client and the server.
        - Just do not resend the avatar I guess?
    - Watch out because the client might already be added to the game, as in, registered in the mode
        - and we always ensure we don't do it twice
        - Shouldn't be a problem: looks like we register the player in mode as soon as the state is WELCOME_ARRIVED
            - and we'd revert back to 

- Let's keep the ability to chat btw
- Corner case: client received solvable vars with a custom arena..
    - ..client requests download
    - But the server didn't receive it yet and another solvable with another arena arrives 
        - We should just ignore all solvable vars, perhaps even with public client settings, and resync completely

- Always scan the part directory
    - Because we might have left a download halfway

- What happens if the current arena changes during our download session?
    - Note as it stands we're applying server solvables as they go
        - We only pause the entropy streams 
    - So I guess we should also resynchronize the server game vars
        - Wouldn't it be easier to stop all solvable streams and just revert client back to WELCOME_ARRIVED state upon resync?
            - And do not interrupt map downloads

- retest hex generation locally on maps

- reset keyboard afk timer when downloading complete

- Note the block size itself is a notification of the file size, we should be able to read from that

- Pausing solvable streams for clients
    - Do we want to though?
        - Some important public client settings are sent through
    - We can easily interrupt just the entropy streams and leave server game vars and public settings intact

- Disable naming maps "autosave"
    - Otherwise it might fuck up maps, autosave will literally be deleted

- Remember to also resend solvable vars once files download

- We need to say which map's file we're interested in! 
    - We can't just use an index and assume the server will be able to understand it
    - This is also future proof as it will let us download all maps from a server if some allow it in the future
- Therefore, the server needs a per-hash content database
    - For now it can just generate it on the go from all loaded maps 
        - The client will thus be able to download content only of arenas that have been played at least once
    - Old maps might get wrong hashes once they are modified
        - But then it's just a case of loading the map again on the server, it's unrealistic to think files will change there for some reason and someone will want to download them
        - We can just say download failed. Server could find the file
            - this will leave the map in the .part state exactly as if we had disconnected, no biggie

able to request per map

- Also which one do we replace if we have a map in both downloads and user projects?
    - Default to downloads right now

- Idea
    - First always download maps to de_knysza.part
        - Only once it's complete move it to de_knysza.new
        - Then after playing, a dialog
            - "Your last downloaded de_knysza differs from the local version. Do you want to apply it? (Upgrade) (Discard) (Cancel)"

- Server config could specify if they allow downloading maps? But should be on by default and maybe not disablable from UI
    - We will always send files one by one, to not have to allocate space for more messages in reliable channel.
        - We'll ask for file hashes, server has to create and keep a map from hash to path when loading any arena
        
    - Purpose would be to save memory space really: The channel for file transmission wouldn't be created

- Transmitting maps and resources (finally)
    - The client's request may come WELL AFTER we have connected!
        - Why? The server might change the map while the client is already in game!
    - So let's even begin by designing AND testing for this case
        - SO - What if we're already connected and the map changes to a non-existing map?
            - This could only happen with a new server game vars

    - The server does not really have to hold the json file in memory, although it might be useful
        - At first we can treat it like any other file
    - First download the json file maybe so that the client knows what to ask for
        - We ask by hashes not paths to be secure, and also because there might be more than one file with this hash anyway
    - So we can have two client->server commands
        - Request map file
        - Request map resource
    - Hard part will be to sustain the connection client-side without crashing in absence of a valid arena perhaps?
        - lol we could first download the json file and read it, only later paste files
        - this is where determinism with just the json file will come in handy
        - although no, it'd be silly to show an incomplete map
        - what about showing the chat etc
            - do we even send chat messages to clients not in game?

    - Reusing resources for new versions of maps.
        - Will naturally happen frequently when hosting playtesting sessions from editor.
        - We don't have to start with a full-blown content database.
            - It's enough to look up the downloaded resource hash from the map we're replacing/downloading new versions for.
                - Will cover 90% of the cases.
    - Interrupted downloads
        - Need to identify that a map downloaded halfway
            - Say, if json exists, map downloaded completely
            - This will play nicely with detection logic for existing maps
        - Although it will logistically be helpful to download json first
            - we might want to just download the json and skip resources for some reason if we want to download it from someone
        - We should at least determine if external resource paths exist
            - Same complexity as we need to read those same resources after all
    - Masterserver heartbeat will have a hash/maybe even saved utc but it will only be for the purpose of filtering
        - Downloading decision will only happen after receiving solvable vars
        - Especially since some servers might have heartbeats every once say 1-2 minutes and it will make us download the wrong map all the more often
            - Although we can't help downloading wrong maps sometimes because by the time we download it the host might change the map as well
                - But we can minimize it
    - The client must essentially be able to request any map from the server
        - Potential security issue? We'll just sanitize the input, easy
    - We'll still be sending solvables etc. to stay connected
        - We just won't show the arena on client screen as long as they are requesting anything
        - Once we're done we will have to re-request a state correction 
    - We should impose a maximum size limit per file 
        - e.g. 16 MB
            - Simplifies implementation because we can just send a single yojimbo block
            - Forces people to be more mindful of the file sizes
            - Good for security too
        - Wrong file sizes will be ignored
        - However we should have a separate channel so that 16 MB isn't allocated every time we connect to an existing map
        - Remember though this is per-client.
            - This will shoot up our memory consumption to 64 * 16
            - I think 8 MB is enough, even free discord has that limit lol


- Note we cannot do the de_cyberaqua vs de_cyberaqua.old swap WITHOUT PROMPTING if there are only 2 versions
    - This way a malicious server could wipe everyone's maps by constantly changing them once someone connects

- Run static analyzer and thread sanitizer at least once after finishing transmission

- Universal additional collision sound for bodies so we don't have to create materials for some basic uses
    - e.g. for bouncer

- Good balance for sniper scopes:
    - When zooming, place a red dot where the cursor is
        - It's only a dot so you don't know its direction
        - And it should also be obscured by fog of war
    - This way when someone's camping, victim won't know the direction of the sniper
        - but they'll have a chance of knowing there is a sniper so it will be fair!

- Exit crashes: there are sound sources declared like:
    - static augs::sound_source tick_sound;
    - They might be causing AL ERRORS on exiting the game.
    - sound buffers are thankfully just in viewables stream
    - there's some in the main menu setup as well although probably less harmful
        - this will stop and exit along with the game thread exiting so should be fine
        - just these statics should be removed
            - we can test if it's the reason on windows, with a longer tick sound

- choose default audio device from settings

- GIF determinism: Note that to preserve determinism while resources aren't fully loaded,
    - gif frames would need to be serialized
        - which would prevent nicely reloading them
    - that is because they're not a subject of streamable, but reside in state
        - gif durations have no way to be reloaded right now
    - even though they're harmless animations, they partake in animation system
        - and can get destroyed when the animation ends for example, which would influence state
    - we should just assume for sanity that all map files are required for determinism, why not?
        - some scripts would be required too, although one could argue they will only be server-side
    - we can always add this gif serialization later if it's really required

- show timestamp of commands when hovering
    - especially for autosave and open project revisions

- Allow resizing by axis only
    - just like position - separate widgets for x and y

- Sound nodes should have pitch/gain
    - Also specify Play once for sounds like on fy_snow 

- Simplify hosting server ui especially once we allow hosting custom maps
    - esp. with that max incoming connections, it's silly af
        - just call it max_players

- Maybe let's have override_domain after all

- duplicate -> clone

- Online playtesting (finally)
    - For showing current running session's chat
        - just have a ptr to server_setup* and call its perform custom imgui for arena mixin instead of editor's
            - This will even let you use TAB and check who's connected while you edit

- Quick refresher of server architecture before we introduce serious changes
    - Why did we want to have session ids? And why in the mode?
        - Identifying by client ids 0-64 is prone to glitches because the client might disconnect and someone else might connect
        - We also want session ids be synchronized so other clients can refer to them
            - The crux here is they can refer to/recognize them asynchronously to solvable (deterministic) stream
            - so e.g. the avatar for a specific session id might be sent in parallel to gameplay
    - Do we need to distinguish between mode player id and client id?
        - I think it's a good distinction
        - Note client setup does not need to know of "connected client ids", he just has mode player ids and their associated existential session guids
    - Then, why not ditch mode_player_id and just always use session_id and map with it?
        - The server would just map player_session_id to its 0-64 slot internally
            - all player_session_id are correctly synchronized through solvable stream because they're held in the mode
        - Pro: We can identify players with just 1 byte instead of 4 (we know it's in 0-64 range)
    - The current arch then makes sense.
        - Modes are convenient way to associate players to session guids through solvable stream.
        - And we want to be able to synchronously identify players with just 1-byte ids.

- Resolving duplicate arena names
    - WILL happen.
        - When you have made a map and you later want to download it
            - We should also skip downloading if our map is identical on hdd
    - If an official map exists, it should always override downloads/user projects
        - A map like this should NOT exist on HDD
        - If we didn't override someone could easily make their own version with transparent walls
    - Therefore:
        - Official > User > Downloads
            - Official is first for anticheat and consistency (so de_cyberaqua always means de_cyberaqua and not some custom thing)
            - User is first because the mapper might want to host their updated version of their own map they downloaded somewhere else
        - Note that with this people could still make their own versions of downloaded maps with transparent walls, so maybe override it 
            - Of course when you host, you should be able to choose any map
            - In particular watch out to not host the downloaded version when you mean to host your updated edited version
    - For security, order of resolution is irrelevant
        - Client should just check map hashes if they match, every time
            - Otherwise a script kiddie will just edit official maps directly
            - So we should send hashes for officials as well
            - And obviously for community maps always as well
    - editor_project_paths must then resolve by name and hash
        - With the nice format for current_arena variable, especially if we want to change it through config/cli..
            - ..we won't be able to choose one duplicate name over the other specifically
                - but are there cases we would like to do that?
                - A mapper could host an earlier downloaded version for a test run
                - Or host their own updated version. The duplicate needs to be resolved properly as well.
            - We should therefore treat user/projects/arenas as a part of the downloads library!
            - This way downloaded arenas will have a _VER1 etc suffixes
                - However it would be dump to send "de_rats_VER1" to the clients when it is not known how it is numbered at the clients end
                    - This applies not just to editing.
                        - If we download multiple versions of the same map, we still have to do this to host one of them.
            - Should we even show _VERx and such in the choosers and project selectors? Making them legit maps?
    - We wanted to enable a flow where one person hosts their map, their friend downloads it
        - then later they edit it and host it as well
        - and the first guy can easily apply those changes
        
    - To resolve downloads/project collisions, we can preffix the name with "!" to denote the map MUST be taken from the editor files
        - This is a one-shot variable and the ! should be stripped ASAP
        - We can do this later. For now we can just disable hosting 
    - Alright but this is still damn ugly
    - Can't we just always prioritize the map in users when hosting?
        - Why would a mapper want to host an outdated version anyway?
        - They'll just ask someone else to host it
        - Well there's no UI reason to disallow choosing the one from 
    - For now let's disallow 
        - Later we can add ! and strip this during net serialization stage
        - current_arena variable will have to be sanitized anyway

- Verdicts:
    - Server-side and client-side arena choice are different routines
        - Client will always look for name+hash pair and doesn't care if it finds it in official/ or downloads/ or user/projects/
            - Note that even if someone deletes official map from the official folder
                - and then modifies it for their own benefit, the hashes will not match
    - Server-side duplicate name resolution when requesting x
        - For now assume user/projects > user/downloads
            - This will work out-of-the-box when we want to request online playtesting
                - The only corner case is when a mapper wants to host a version of the map they've downloaded but we'll handle it later
    - It will be mostly transparent to the user because they'll choose the map from the arena selector
    - current_arena is literally just a *hint* for the client where to look for a map with a given hash
        - will be sent through server game vars too probably
        - server_vars (note: not SOLVABLE vars) will have a load_arena_by_path
            - This will be the setting set by the arena selector and this will load the arena once
                - Nothing stops us from loading a map external to the game even although that's not recommended
            - the selector will also set the current_arena to the filename (minus say .old and other possible version modifiers)
    - Security concerns
        - Of course the client should disallow downloading a new map if it's official but it's only for convenience
            - since someone can delete arenas from the official folder
                - This will not change the fact that the newly downloaded map will have its hash checked against what the server requires


- The semantics of augs::maybe isn't really one of "is it enabled" but more "is it specified"
    - which is why it would make less sense to use it for fog of war for example
        - because this mechanic exists at all times - we can just temporarily disable it
        - which is why there should probably be separate flags for it

- Prepare comprehensive official resource collection
    - What do we make official?
        - Vent sprites certainly because we already have an official vent physical material
        - Maybe some complex-shaped stuff
            - The steel crate
        - Entire garden biome

- Later do an interface for showing nodes using a given resource

- quick_test-exclusive property: Respawn at team spawns
    - Disable by default

- Fix maybe how point markers are highlighted because the white aabb edge is completely invisible

- Honor playtest spawns but maybe as respawn points only and spawn where the camera is?
    - BTW WE NEED quick_test_spawn to properly spawn characters during quick_test!
        - Not necessarily. We can have a playtest_context

    - For quick_test, it is okay if all client players appear in the same place as host
    - the host can anyway predict where their "enemy" is and it's a nice way of meeting in the same place
    - It is also okay if the players respawn right where they died to keep the "fight" going
    - with this, there's very little usecase for creating your own quick_test_spawns, but if you want, there's nothing holding you back
        - position in layers will determine the first order of respawns likely
        - by default they can only be used to *respawn* instead of spawn
            - playtesting settings could have a tick to spawn at quick_test_spawn instead of camera center
    - for now we could disable them
        - nah maybe let's leave it but just rename it

- btw in a playtesting session in quick_test mode, all clients should join the opposing team
    - for balance of course! The mapper knows literally everything about the map

- area_marker/area_marker naming consistency
    - though it's not significant in json state

- Turbo-simple Online Playtesting Ctrl+P implementation
    - Can literally create a server_setup making an editor_setup backup like when project selector loads an arena 
        - (this is so that the same map ends up selected in project selector if the load goes wrong for some reason)
    - Client knows it's connecting to an editor playtesting session
    - This also replaces the flow where you're "Constantly online" which may be uncomfortable to some
        - You're only online for the duration of the playtest
    - We might begin with this also before moving on to a proper implementation which will be way better
        - We at least want the editing guy to be able to chat with the people he disconnected with otherwise they'll lose interest quickly when they're in the offline mode, perhaps it could still work while they're on a voicechat etc
        - and it will be cool to show connected people's positions in the editor

- A node should could an option to make it non-physical?
    - So it can be spawned just as a decoration because sometimes the thin walls might be used standalone physically, or separately over the layout defining colliders
    - Some physical nodes might also be overlaid transparently (?)
        if we determine we want our own physics for an aquarium for example
    - Alright so the aquarium surely should have the option to not create the physical aspect
        - 1) because it might be somewhere off-screen and it'd be a waste of resources
        - 2) because it might be used in a layout-defining context and it might conflict with the walls
    - In legacy editor, we have instances of the same image being used in both physical and non-physical context'
        - Notably lab_wall

- Serializing maybes.
    - Positive-default examples to consider
        - fog_of_war: false
          fog_of_war: {
                angle = 1800
                size = [1920, 1080]
          }

    - Old:
        - I'd think of a nicer way to serialize augs::maybe<> in json
            - Either have enabled = true/false inside the struct (in practice these structs are guaranteed to have no member like that because otherwise what would be the point of maybe<>)
            - or at least name it disabled__organism vs organism instead of enabled_organism
                - actually "organism__DISABLED": because we sure as hell won't have capitalized members and it's visible clearly
    - Things would be way easier if we assumed that no maybes will be true by default
        - Then we just have the intuitive OFF_ and we don't have to handle DEFAULT_
    - It would honestly be really stupid semantically if we allowed enabled maybes
    - Then also we don't have to worry about serialize_disabled flag
        - because being disabled will never be 'different' state so no data will be lost
    - The reason why it makes no sense for a maybe to start with true, at least in the editor context, is
        - because disabling it then would mean 'overriding' it to another value
        - this is semantically the same as having an implicit value D that is taken when maybe is disabled, and then switching to default F when it is enabled
            - with maybe "enabled" by default, that D is maybe::value and F becomes the implicit value
        - so what about a switch like draw fov? Enabled by default and it specifies angle. An actual game mode var.
            - Nicest way would be to have an augs::maybe for it actually
            - then to disable it we just write "fog_of_war": false
                - This is not an override technically, so we should have separate vars for it like 
                - "fog_of_war": true/false
                    - In any case we should prefer this instead of OFF_fog_of_war 
                - "fog_of_war_angle": 180
                - etc
                - although here we're confusing serialized mode data with actual config vars which will be designed to look nice
                    - though if mapper can override them per-mode it would be nice if these at least resembled it

    static_assert(!std::is_same_v<bool, typename Field::value_type>);
    We don't have to specialize for bools.

    We can always just pass null type to signify the default

    Maybes are tricky.
    If the value is the same but is_enabled differs,
    then we should write something like:

    "maybe_field": {}

    or:

    "OFF_maybe_field": {}

    To only signify whether it's enabled

    If is_enabled is the same but value differs, 
    then we *need* to write it as if the entire structure differed.
    We're redundantly passing the information on whether it's enabled, though
    In that case we should do:

    "DEFAULT_maybe_field": { "different_field": 2 }

    Well, if you elect to not write disabled maybes at all,
    then what if one is by default enabled?
    We could also do

    "maybe_field": false

- not sure if we need to set the stamp hash when reading
    - This is basically just for the editor to be able to redirect missing resources
    - Game won't use it even if the resource is missing
    - i don't think so, they're probably reread only after the entire redirecting work

- Modes considerations
    - Replacing freeze_secs etc with freeze_time
        - More intuitive for cs-natives
        - Time is connotated with both the unit and the event
    - Remember to set defaults in serialization (isn't done yet)
    - Regardless if we'll later have mode-agnostic parameters for easy setting from CLI like mp_freezetime
        - It absolutely makes sense that we might want to override them from modes
        - They don't even have to match all possibilities, they can but they don't have to, we can arbitrarily enable the most obvious ones from the GUI
        - At this stage it's only important that we name them correctly
    - Some properties WILL be mode-specific. And it won't make sense to specify them in the general variables.
        - Like starting equipment for playtesting - non-transferrable to anywhere else really

- Treat space as _ when filtering
    - At least where filtered stuff is path like
        - But will be applicable pretty much everywhere honestly

- Modes
    - Maybe think this through before we do serialization
    - Could be project-specific special resources
    - As for showing only changed properties, this is a problem specific not just to nodes and we'll have to tackle it anyway
    - Instead of having bomb_defusal_no_warmup etc.
        - just have a flag "enable_warmup_in_editor", false by default
            - Same for freeze time and starting money!
            - Note that we don't even have to include these editor-specific variables into actual rulesets
                - Because they will be rebuilt like everything else!
            - These editor overrides should be at the very top, later separated by ImGui::Separator
                - Actually put these overrides in arena settings!!!!!!!!! Not in specific modes
                    - We want this always available
                    - Maybe even a separate tab, testing
                - Could be called testing settings
        - Similarly with playtest mode, just another set of variables for spawning if it's in server
            - or maybe playtest spawn can specify a flag server_test_only = false
        - This way we'll just have two rulesets most of the time (one for playtesting + one map specific) which is good
- Do we want mode/mission duality like in CS:GO?
    - The only reason would be if we'd realistically want one thing to be playable either as a complete rounds with economy etc.
    - I say no
    - I think they only need it for construction mode because it can be combined with e.g. ctf or tdm 

- We can do resetting to defaults later
    - Let's highlight changed properties in green and allow reset under right click
    - We'll easily just pass vector of hypothetical defaults

- We could attach lights to physical objects
    - Even automatically when they are spawned over a dynamic object
        - so that we don't have to resolve references etc
    - This will work with static bodies too because then they just won't move
    - Yeah, we should do this totally
    - light object could also die with its host

- We could delegate scene rebuild to a separate thread to avoid freezes

- Another reason why most official maps should only use official resources is..
    - ..textures and sounds won't have to be reloaded!

- Rename some aquarium sprites in accordance with the field names in as_aquarium in prefab node editable
    - except maybe for water_overlay

- fix Clear button in resource chooser, it's going off-screen

- Note that to selectively rebuild entities for prefab nodes..
    - ..you'd have to selectively modify scene_entity_to_node which would break it
        - we'd need a proper unordered map for it which will give a slight performance hit
        - so for now let's just skip the hassle and rebuild the entire scene
    - but hey, it's not a problem just with the prefab.
        - even rewrite_last_command calls rebuild scene! which makes sense because editing nodes has to be reflected on the scene immediately
            - so tweaking will be expensive as well
    - for now let's screw it and just rebuild it all on any modification to prefab

- Geometry editing could be available only on the stock collider nodes, will also have arbitrary_collider resource
    - We could be able to merge them too
    - If people use these stock colliders extensively, does it break our future plans of arbitrary geometry editing?
        - Not really.
            - We can have a "merge" function available only for stock colliders
            - This will change their type to e.g. arbitrary_collider_wood or arbitrary_collider_metal
                - Even if there are conflicting materials in the original collider nodes, this isn't really important as you can still change it
                - Vertices can then be changed arbitrarily
                    - Editor could even hold original collider info until at least one vertex is modified
                    - Also the stock box/triangle collider nodes could be transformed into arbitrary collider the moment their vertices are modified as well
    
- Consider adding "+1 times" to resize calculation
    - Also automatically "not tile" *during resizing* if it's less than original

- Map caching/compilation
    - *Let's NOT keep cache files inside map folders.*
        - Would add security overhead: someone could upload a map with a malicious cache binary
        - Instead let's keep it in game's own cache folder
            - like: cache/arenas/official/de_cyberaqua/compiled.bin etc
        - also way cleaner, less clutter in the map folders and less stuff to blacklist later for transmission etc.
    - This can be an easy bin file in cache
    - Or even an adjacent file
    - The bin file can hold a stamp inside with both the game's version and last_write_time
    - so we don't even need a folder for that, but it might be nice

- First sort out physicals and handle the aquarium spawning at the very end
    - Apparently resizing physical bodies doesn't honor the custom physical collider.
        - Only if it's a box

- We gotta wrap it up

- We can ship editor as soon as we reproduce fy_minilab
    - So people can play in the meantime
        - and I can start redoing other maps

- Filters window
    - Visible
        - (Checkboxes) Backgrounds Physical Foregrounds
    - Selectable
        - (Checkboxes) Backgrounds Physical Foregrounds
    - Let's just add an eye to the toolbar
        - Later optionally "Filtering" to View

- Cover ground neons per-entity?
    - Only foregrounds will need it on a second thought
    - Consider making anything foreground cover the ground neons by default, but I don't know how we'll do it
    - When something needs to cover ground neons per-entity it's usually static
        - So we can do the job with a static neon occluder
            - Although that's PITA when we want to move the entity so maybe let's have per-entity switch after all

- Ctrl + Scroll could move nodes up/down
    - We could save it in Edit menu (Move Up in Layers) so we don't forget

- Server playtesting
    - Once we hit clapperboard once, "Play" could automatically refresh the server
    - Let the other player still play once we go back to editing mode
    - Other players can always spawn by default where the host spawns
        - will collide if some two connect at the same time but doesn't matter
    - Then their positions can be preserved when they refresh the map
        - That preservation can be done server side easily because server_setup will just receive a signal to refresh it 
            - and will readd all players to the test mode
    - can show an ip address to copy next to the clapperboard
        - or even copy it automatically on pressing ctrl+p when it's first hosted
        - the clapperboard might actually just be to host the server and play will enter the game
            - when ip shows it will be a nice feedback that the server was hosted

- Mirroring works differently than flipping for animated sprites, but that's probably because we don't have flip property in dynamic decorations

- Playtesting
    - Alright let's do this, it will be rewarding anyway
    - Separate setup?
        - Hosting a server will require saving the changes first because we'll actually switch to server_setup
        - Normal playtesting can easily do it in-editor as it will use just the test_mode so won't even need to save changes
    - Server: To notify or not to notify
        - Maybe notify by default and only hide notifications for passworded servers?
        - For now it would be good to always see if someone's making a server so let's notify by default
            - Won't really be accidentally 'spammable' as you'll have to save the changes first and maybe fix warnings/meet the conditions for game mode etc


- Note that domain could be overridable on a per-node basis
    - This is because physical domain involves strictly more properties than both foreground/background (which are the same)
    - So we could enable this option for nodes of physical resources

- Okay but what are the remaining problems with this prefablike special objects design?
    - If it's area marker, it will not correspond directly to aquarium's physical bounds
        - Not big of a deal but still, especially if aquarium is meant to be layout-defining
        - On the other hand if we make the organism area the actual physical bound it will be confusing that it's larger than the actual bound for fish
            - However most likely it will not be noticeable
            - We'll just have to hide rendering of the true organism bound marker which would be ugly
    - Okay I don't think we have the choice, we just have to accept physical bounds will be larger and go with the first, simplest solution
        - Someone might want to specify different wall sizes anyway so for layout defining aquaria they will have to figure it out on their own 
        
    
    - Other is where do we spec the default resource setup for the aquarium walls glasses etc.
        - The defaults will obviously be official
        - and I think we could technically set them in editor_node_defaults, but maybe we could store and preset them somehow somewhere (?)

- Couldn't we implement "special resources" in terms of "node templates"?
    - The previous "special resources" design was really just a response to ours not having implemented enough node/resource types
        - With all organism entities, particles and organism areas in place, we could make aquarium just an instantiable node group
            - We can still call it prefabs for what it's worth
        - it's just it will spawn an entire layer
    - No no no, we lose all the parametrability we wanted to achieve
        - the deal was that we wanted it to be instantly resizable for example
    - We wouldn't be able to have a simple json type = 'aquarium' with just size and be done with it


- Maybe ask during Q&A if we should indeed reset the tab or not

- Actually map could have in its settings two checkboxes:
    - Include all invisible layers
    - Include all invisible nodes
    - This way we could edit some specific fragments in peace but have them always finally be included
    - I believe this should be off by default as it will only become useful as the map grows really

- Also we have all required special objects for aquarium
    - Well.. except for the organism bound marker

- Now that we're dabbling with special items..
    - Let's spawn all necessary things for a bomsite
    - The map should technically support as many gamemodes as possible
        - Once we have many gamemodes we'll let the map creator select which game modes to initially support (they have to choose at least one, create button will be disabled if there's none)
        - We'll have separate layers for objects pertaining to these gamemodes
        - gamemode_common 
            - buyzones
            - team spawns
        - gamemode_defusal
            - bombsite a
            - bombsite b also to let someone know intuitively there can be more than 1
            - bombsite c too since we already have an icon lol
        - gamemode_ffa
            - neutral spawns

- Also let's similarly have colorizeable particle streams with the initial color in node_defaults

- Consider renaming all "colorize" to "color" in the editor
    - Once we're doing serialization tho

- Once we're remotely close to MVP - EVEN WITHOUT A GEOMETRY EDITOR YET!
    - We should start rebuilding fy_minilab pretty soon
        - This way we reason what is really needed at this point
        - Per-sprite geometry editor will obviously be necessary too but we'll port it
    - We'll need a way to mass export image parameters like neon maps
    - We could make it way easier: just export neon maps as pngs, we don't care about rest of the parameters
        - Except shapes... but we could reapply them, shouldn't be that many
    - We might want already export to json from the old editor
        - We'll import in json in the new editor
    - It's a cool idea - next to a png file there'd be a file.png.json file
        - These would ONLY be defaults
        - So they'd be values after pressing Reset in resource view
        - It would make it extremely easy to port resources from one map to another
    
- By the way let's thoroughly check which officials we want to list
    - We might also want to change some names for accuracy so that it's future-proof
        - Will break maps but only if we wanted to press "Update official content" (in player window) on them

- We can finally implement game atlas culling now that we don't use it in rendering
    - Remember ad hoc won't need culling because we'll just use tiny tiny icons
    - Only the inspector gui will need using ad hoc

- Also add insects and garden sprites to officials, will be handy to have all biomes
    - Maybe it will motivate is to implement culling
        - which will only break the official firearm rendering for now
        - And might break animation rendering if we use the game atlas

- We were worried that once we move to gif-format for officials, we won't be able to skin e.g. new stances as usual
    - But there won't be official maps to read from anyway
    - So we'll have to prepare an intercosm for skinning like this
    - And then we can just pass the generated png file paths as always
    - In any case we'll probably port it somehow or even use another tool (?)

- Gral gif is colorized yellow when neons are enabled, probably due to wrong light calculations
    - We should probably check on windows first

- Randomization like on assault rattle should be at rendering time instead of animation time

- Btw first thing we should do for atlas optimization is have a separate neon map atlas

- Editor sprite resource will have a vector of frame metadata
    - It will always expand and will never implicitly shrink if there are suddenly less frames in the gif file
    - But we don't necessarily even need it for now, at least until we enable per-frame editing
- Actually how is size currently read?
    - We will literally read the frame count in the same place
    - Size is always overridden as well in the resource
        - It's just that nodes can override it
    - No need to reuse get_size because it's actually implemented to be fast

- Okay, how do we unpack these animations?
    - The problem: viewables access specific frames as assets
        - We could virtualize the paths
        - This would break the old editor a bit
    - However!
        - It doesn't matter for cosmos logical assets
        - But we regenerate viewables in editor too so it should be correct
        - Path virtualization probably the way to go like .gif#1
            - This will only happen in viewables
            - What do we need to fix for this to work?
            - Basically atlas generation
                - Uhh it also breaks neon map generation
                    - Alright fastest and easiest and cleanest would be to just save the gif in caches instead of virtualize it
                        - But at which stage?
                            - Given the path in cache would be deterministic, we could unpack it in atlas generator
                            - The files do not have to exist right away, they will be generated by atlas and thus will exist once they're needed
                            - And ad hoc will anyways read from the gif file directly
                        - Not necessarily the most performant with the encoding overhead but screw it?

                - Atlas is already able to read gifs
                - It just needs to repoint these virtualized paths to a proper atlas sprite
            
- For officials, we can query meta lua files to determine how many frames in gif there are
     - Unless there are frames without metadata
     - Okay we could quickly query the gif info, probably the best way to go
        - Nah actually I think every official has meta lua
        - But we might need to query this either way for the custom gifs

- We can query the meta files for officials, but what about custom gifs?
    - Maybe we should handle them first
    - Virtualization will be necessary either way
    - We'll also need to know the number of frames
        - Even just for editing props (which would normally be a very special case but still)
    - So we'll need to know the number of frames at asset creation stage in editor logic
    - So we'll probably need to know the number of frames either way
    - We can read the number of frames as metadata and store in filesystem resource next to thumb id
        - Later we can even use ad hoc atlas to display it to edit some per-frame properties

- We really shouldn't worry about the old editor for now, we can wait this much
    - Only important that we don't break the abi for reading current binary maps into the game if e.g. we need to push a quick unrelated bugfix
- In that case we might need to leave the png files that end with 1 2 etc
    - But we can freely modify the official flavors

- Animations
    - Okay so we just need a means of translating some editor animation resources to proper cosmos resources
        - Contrary to classic editors we won't make animation a vector of sprite resources
        - It will instead keep a single animation resource with properties for a single sprite to be applied to all frames
    - How do we plug them into object-per-sprite system?
        - Can't we just make it a sprite?
    - Technically for editing guns or walking animations we'd need to skin/edit state of every frame 
        - But for environment objects we should just make it abstract
    - Used only in a) animation component or b) particles defs
        - animating decorations/organisms (animation component)
    - why can't we just create animations for viewables as usual?
        - we probably can and will
    - We only wanted to templatize the particle types because we need a separate means of keeping them in the editor state
        - could be introspectively assigned
    - What about pathed assets?
        - Do we want to keep separate forms of animation, i.e. separate pngs and gifs?
        - I think just one will be enough
        - It's good if we force small number of colors anyway
    - How do we create the rest of resources like officials?
- So problems to solve
    - For adhoc there can be a separate system that just keeps track of the animation and feeds proper changed coordinates
        - Without virtualization on the atlas generation level
    - We won't recreate the atlas every time, we'll just pass different coordinates
    - Okay so this is done


- Aquarium creation logic is important because otherwise we won't be able to faithfully reproduce de_cyberaqua and fy_minilab

- Finally test that gif->on-scene animation pipeline
    - And maybe mass-gifize some png sequences like fish
        - Even just for the sake of nicer view for official resources

- Static collider entities
    - static_collider
    - just a physical material
    - will arguably never need some specific interaction logic as this could simply be in a separate non-physical entity
    - it's not meant to be much of a dynamic thing either
    - Resizing/Rotating will just reposition vertices
        - Or will it? Maybe let's just apply these parameters like with normal sprites
            - Although size is non-applicable but we can still have scale
        - If we ensure standard objects *cannot* be selected along static colliders, we can run some custom resizing logic
    - Resizing/zooming might complicate our snapping logic
        - That will only affect how we calculate the offset though, it will always be just a translation
        - But still we'll probably want to preserve that information about how a shape was initially

- A radiobox-like tabs for "Object layers"/"Collision layers"
    - This is the final boss
    - A simple on-scene vertex editor will be VERY handy and it won't really be that complicated
    - It will let us avoid implementing the whole "detect neighboring static walls" and just let the mapper map it out themselves precisely
    - Allows for some cool skinning options
    - This could even be a tab in layers view?
    - Although theoretically nothing stops us from having these static walls as nodes like all others 
        - Still it might come in handy to be able to instanty switch between these two modes
        - It will just make these collider entities visible or not though
        - And at most change some default mouse behavior
    - A full-blown on-scene geometry mode will also be essential to have soldat-like irregular maps
        - Lack of this is what made de labs so squary
    
- Layers with all-default values should be deleted once the last element is removed

- Once special sprite effects/continous effects are live people will rarely use playtesting, obviously we sill need to do it

- Asset-based Icons for special resources
    - Do we take it from the game atlas?
        - Technically when we prune the atlas it might be missing
    - For project-based resources, there will always be a resource with the sprite from which we can read 
    - For officials we could refer to the initial scene
        - Problematically this won't necessarily be in the game atlas
            - Especially if it gets pruned
            - I wouldn't prune officials though because technically they can always be requested by some scripts
    - Taking them from the atlas will work and I'd avoid pruning the special resource related assets

- For now we can leave the audiovisual speed on 1 because it doesn't break anything and looks cool
    - We could even simulate a passing global time since I think that's what the rainbows are based on

- Don't list physical materials for now in the filesystem
    - It will only make sense once we can view them or create new ones
    - for now it's enough they'll be choosable from some list
    - Once we do list, a simple tooltip when someone tries to drag&drop e.g. a physical to a scene
        - Physical materials cannot be instantiated as nodes.

- In case we want more standalone particles we'll just add them
    - As for singular effects.. technically these are separate resources
        - Thing is something like a muzzle shot effect or any burst is different from environmental which should be normally viewable through official resources
    - We shouldn't worry about it for now, we won't be creating a particle editor for a while still
    - Does the same problem happen elsewhere?
        - Also, materials
        - So we could just have separate categories, sound effects and particle bursts apart from "decorations"
            - decorations pretty much imply they're static
    - Sounds would technically be similar, there are bursts and ambiences
    - However someone creating an environment in the editor won't bother with "bursts" and "sound effects", they will only deal with these environmentals
        - Well unless they're making an interactive object somehow


- By default all official resources will be created. What do we do with the redundant textures that inevitably get included?
    - We need to prune them once on cosmos generation stage
    - I see even de_labs2 includes road sprites which technically sucks
        - but not a problem for now since it's worked like that for a while
    - Yeah at some point we'll need to run a dependency graph and remove viewables that aren't referenced
        - But we'd need to do this anyway even if we didn't create all official resources at abi level


        - Reusing official content creation on ABI-level
            - Pros
                - Editor ready sooner
                - Finer control for now and won't screw up creation process
            - Cons
                - Missing out on easier creation process for official resources
                    - Though one could argue that doing it at abi stage is more... immediate? (Won't have to create translation logic for a new feature we want to test)
                - Won't be able to reference it/properly visualize it from the editor as a resource?
                    - WILL be able to reference it actually
                    - and will be able to easily create an ad-hoc icon logic
                    - This is the only means of visualization we need
                - Won't be able to view properties
                    - It's not important
                - Also won't be able to create a custom resource basing on it
                    - Unimportant too, will encourage creativity
            - Note that even if official gun creation happens in abi...
                - ...we can still have custom gun resource creation logic without being forced to use it to create the official resources


        - Can we do it though?
            - Problematically, guns/particles/materials etc will refer to official resources which will be reference-able from the editor
                - Can't we just visualize and somehow associate them? Given their properties won't be editable and don't even have to be viewable
            - E.g. we wouldn't be able to do it with sprites because we wouldn't even be able to list them
            - We already have a separate 
- We could technically hold enum tags in editor's sprite resource struct
    - If it's set we know it's also official
    - However we'll already know it because official resources are in a separate struct

- Could show a full preview when hovering specifically the icons in filesystem

- For particles we might just templatize the particle effect by the image id and animation id
    - We don't even have to explicitly instantiate particle types as the defs are included in hpp

- Handle throw_through later, we'll make it just see-through for the moment
    - we'll have a separate filter for that probably

- I would say ultimately physical material is for damage sound/particle effects
    - Although maybe let's leave that ricochet there after all

- What we'd really want to avoid is to having to specify collision sounds twice because we want different restitution/density on say another wooden body
    - max_ricochet_angle is likely something we'd like to unify, though


- Materials
    - Can't we for now have a set of defaults without it being editable?
        - This could be officials

- Technically, it's the nodes/entities that should have modifiers, not resources/flavours
    - We need to fix the particle decoration implementation
    - but we need to preserve bincompat for now
- Similarly sound_effect_modifier is more like the default properties like the particles' emissions, a modifier would just be gain or sth

- Another special resource: materials
    - But only physical, we won't create a separate material for grounds as it's just a single sound

- We won't forget the possible actions since all of them will have key shortcuts
    - But we need to describe them in tooltips over the toolbars and buttons anyway
        
- Watch out for pixel imperfections. List of possible fixes:
    - We've deleted aabb calculation from the mirror entities command.

- Once we come to prefabs, it should be easy to rewrite these transform commands to operate on nodes
    - We only really need a way to access sizes and positions (as shown by the data that's backed up for undo)

- It seems to me that it's not the worst pattern - all transforming logic working on the generated scene, as opposed to abstract node entries.
    - This is because node transforming logic works on *apparent state*.
    - E.g. for selector, with special stuff for prefabs you can't be sure how something is going to be generated on-scene
        - so you'd anyway be working on entities there and reading back

- Now for a fun question: can commands really contain entity ids?
    - Since they're not saved anyway...
    - This would spare us the initial entity->node translation
        - which would need to be translated back anyway
    - I guess entity id creation is deterministic
    - A million dollar question: how do we revert it? It would seem dirty to save just the state of entities and then read-back on undo to the nodes
        - Because there's no guarantee we'll go to the original state on revert
        - Wouldn't that work anyway? There's no magic involved

- Remember that porting those moving/transforming things will be the hardest.
    - The hardest work will be done by then
        - next hardest is pretty much just state i/o
        - then duplicating/deleting etc

- Now about the moving and transforming...
    - It would be extremely complex to port it to the node structures
    - Problem is it works on individual entities
        - What about prefabs?
        - Prefabs will possibly have a completely different resizing/transforming logic

- Hang on. If move_entities_command accepts a single delta, can't we use it to simulate moving all of the entities (prefab included, just making sure that it's selected in its entirety)?
    - Resize is harder

- After all we could use the legacy commands to let the user "choose" so to speak

- reference_point is just mouse position in the resize command
- Translating back from resize command should be relatively easy
    - It's clearly only touching sizes in overridden_geo + the independent transforms

- Let's not worry about prefabs for now


- Looks like the sound icon selection is broken in the original editor too
    - Light icons work so let's first test that

- Don't worry about atlases being too large with there being too many official assets
    - It's enough that we generate miniatures in memory for the ad_hoc atlas, this should be trivial
    - And we'll implement the occurence detection anyway so that only needed textures are getting into viewables
        - The only problem with that is that when we drop a new object onto a scene, the atlas will regenerate
        - But that's really a small price to pay


- Can we truly assume that there is one entity per node and vice versa?
    - What about prefabs?
    - Well, this is pretty much only about whether we want to rewrite the entity selector to use nodes
    - But I think this would only complicate stuff
    - We can translate it to inspected node ids with do_left_release
        - just like selected_ids is assigned

- Dragging from layres gui will duplicate
    - This will also play nicely with prefabization
    - A layer will have an option to prefabize
        - It then changes all layer objects into an instance of a prefab
        - And creates a new prefab in special resources
        - But that instance can too be duplicated right away by dragging it
        - "Prefabize visible" which only takes the visible ones

- I'm thinking a weapon should be a special resource, not a sprite

- Okay, how do we squash all the:
    - Non-file resources
    - Official resources
    - Into this single window?
- It would, in the end, probably be the easiest to just have separate files per all resources
    - Although we have to anyway enable creating new folders for this easily from the editor
        - If we do, then there's no point in keeping these folders physically?
            - maybe there is to not have another layer of separation
            - We'll just have separate official and non-official
            - Plus you can group the prefabs and all related data together in the filesystem
    - Like in unreal, we could have a separate container called "Specials"
        - we'd then have four

- Actual json files for lights/prefabs/particles:
    - Pros of 
        - Easy to group together
        - Easy to edit/version externally
            - Well, it's versioned in project json as text anyway
    - Cons
        - Atomic save becomes harder
            - Harder to know whether a project state is valid
            - This won't be so uncommon since people would be able to move stuff in the filesystem
        - Have to hash these again

- I think for now we'll just go with a linear list of special objects
    - Future compatibility considerations?
        - Paths can be easily added as an additional parameter later
    
- In UE literally:
    - de_cyberaqua
        - gfx
        - de_cyberaqua.json
    - Official content
- Well, we can use separators anyway to not create another level indentation
    - Have two ticks actually to freely choose whether to show either 
- Alright but question remains how do we fit specials in there?

- Alright
    - A tab under the filter:
        - [Project][Official]
    - Filter, if active, searches both

- For Lights, Prefabs selectables etc just make a single Plus sign to create new
    - Dropdown menu appears

- Or a separate window for Specials?
    - Pros:
        - Doesn't clutter since we don't use it much
    - Cons:
        - Can't easily search for everything which sucks
- The only real con of an integrated specials view is that it's unintuitive having + sign above resources when we can't really add any new

- Alright let's keep it in one window for implementation simplicity if not anything else

- Introduce iconed entities (nodes) before implementing selections?
    - So that we may keep most of the logic
    - In that case we have to implement "Special" resources beforehand

- Sprite: nazwa noda (bez .png)
    - Show resource
- Albo pokazujemy od razu propty, ale tez show resource sie przyda

- W miarę wcześnie bym zrobił jakas prosta wersje tych multiple selections
    - Czemu? Zeby potem nie musiec przepisywac od nowa wszystkich perform_editable_gui
    - ja mysle ze w komendzie po prostu wektor idow + calosciowych obiektow zamieniaczy
        - a tam gdzie perform_editable_gui po prostu:
            - najpierw sprawdzasz common value i nadajesz kolor
            - potem jesli edited, to robisz fora po wszystkich edytowanych i ten property ustawiasz
            - mozliwe ze tu jakies makro wejdzie bo tego pola nie przyjmiesz jako argument funkcji
                - tym bardziej jesli to jakas tablica bedzie
                - tylko z tablica raczej bym nie robil makr jakichs tylko moze lambde na operacje?


- Ctrl+Z should properly interrupt the drag&drop to scene
    - Redo won't be do any harm as it will be the newest command anyway

- First let's convert the project to cosmos so we can see what's happening
    - Co z oficjalnymi?
        - Okej, ostatecznie nie bedziemy mieli funkcji ktora tworzy oficjalne bezposrednio w intercosmie
            - Tylko jedynie wypelnia ten resource pool bezposrednio w reprezentacji ktora potrzebujesz
    

# Creating the viewed cosmos from the project state

- When drag and dropping on scene, we could either
    - Preview the pasted entity live by refreshing the cosmos constantly
        - This is cleaner and is better UX
        - But less performant?
        - And how do we determine the layer?
        - This could be a continuous create node command that only changes the position and target layer/index
            - just like widgets are continuous
            - It's only first posted however once the mouse is over the scene
                - and would have to be deleted if we go back
                - we'll later worry about recovering the inspector state
            - it wil properly show the target layer position too
                - we'll need to scroll to it though
    - Or just show a ghost that approximates the position


# Stuff

- One-click could select it, double-click toggle collapse?

- Eye would indeed be better on the left, to not mix it with the object icons

- There are many possible actions upon the layer:
    - Rename
    - Collapse
    - Navigate to the center
    - Select all

- One-clicking a node certainly opens an inspector on it
- Double clicking could either navigate to it on scene or initiate rename

- Renaming should be quickly accessible to encourage it

- Multiple selection
    - Generally we'll need it for easy moves of nodes between layers e.g.
    


- Note we MUST use "undo_last_allocate" instead of "free" for nodes
    - Because freeing and then allocating on redo might possibly invalidate references earlier in history
    - Similarly for create/delete prefabs!
- Similarly, if we are to clean unused resources we must also invalidate history
    - Or we can make the clean a proper command! Then not necessarily.
    - But wait, what if we clean, then alt-tab with new resources, and then hit undo?
    - Then the undo_free will fail
    - So invalidate history that is
    - Makes sense because that's the only way to ever delete resource data from a project


- If no object hovered while placing an object
    - If we remember the last layer to which we inserted or interacted with at all
        - Insert to that layer
    - Otherwise create new layer and set it as the last one we interacted with

- We want to be able to select layers to duplicate them potentially

- next
    - layers gui
        - tiled ma dokladnie to co wymyslilismy wiec git a zrobimy nawet lepsze
    - simple resource I/O
    - only then some simple cosmos conversions
    

# Scavenging from the legacy editor

- rewrite_last_change
    - In order to keep command api simple, we might want to have a concept of a "preview command"
        - instead of adding separate functions for "rewrite last change"
        - so until the command is actually finalized, just undo it and redo with new parameters
    - Do we need a separate preview command?
        - We can just keep a pointer to the last modified and not even dereference it, just check
            - Index is a better guarantee due to reallocation

- has_parent in history command - I think not, let's just have singular commands and properly have separate classes for multi-commands
    - it would make for a very confusing ui
- Alright but we'll still need to reuse commands
    - e.g. in a delete layers command, we'd want to use delete nodes too
- No probs, we can just keep a command inside a command
    - and then call redo/undo on the children manually

- As for history, for now we could just save snapshots instead of addresses?
    - although that shouldn't really be too complex
        - well, it becomes complex with various containers
    - We can optimize it later
    - The only problem is that we have other meta that we don't want to be editable

# Everything else

- Screw resource_hashes.bin for now and let's always rehash on launch
    - We won't be writing the timestamps to json because we decided we'd put it into the cached resource_hashes.bin just for this purpose
        - But it only really becomes necessary at scale
            - Can't be too long anyway because it will happen whenever we download a new map
        - We can easily add it later
- Since stamps will be empty (default) they will get recalculated after first call to on window activate - which is called right after loading the json file
    - To be sure we can just not read the content hashes from json (since we'll be doing i/o on resources manually)

- Let's go the easiest route right now and just write all resource hashes and timestamps to json

- Summary
    - Separate pools, official outside editor_project, inside editor_setup
    - Id has a bool whether it's official, a general dereferencer
    - map of path_to_resource is calculated only on the rare occasion that it's needed
    - Do we hold file_hash and path inside structs themselves?
        - It's I guess more convenient
        - because we want to serialize them this way certainly
        - If that's not a problem we can hold a path inside the object, not as a key
        - I think paths as keys look cleaner, there's also one line less
            - it clearly shows it's meant to be unique
            - especially since internal resources will too have names as keys
    - EASIEST: json_ignore_fields. Runtime. We won't get a compile error because a string is serializable
        - screw performance hit, it won't do anything
    - probably augs::json_ignore with an explicit operator
    - can't we just inherit the relevant fields into the full resource? And then cast, I don't know
    - setting path to default so that it's ommitted sucks because it involves copying
        - would be better to just make a runtime if
            - why not then? this is by far the cleanest solution

- Alright so we need to somewhow ignore the "path" in resources
    - Nothing else really
    - e.g. for pseudoids we can just manually read_json from that object into a separate variable
        - and construct an unordered map on the side for just the duration of the json reading function
    - well then, just a struct augs::json_ignore or path_holder?
        - makes code uglier so i'd avoid it
    - we thought about constexpr == const chars but it probably wont work

- Alright so we also don't need to hold the pseudo-id "id" but we need to write it
    - We can write it easily separately
    - the problem is if we need to hold something and not write it, so the path

- Separate pools also makes rescanning for filesystem changes easier because we're not looking up irrelevant data
    - Yeah, so we certainly want separate

- std::unordered_map<augs::path_type, editor_resource_id> path_to_resource;
    - This is really only relevant upon:
        - i/o
        - remapping when rebuilding filesystem
        - so do we really need to keep it? We have to also keep it up to date when allocating etc...
        - I don't think so


- Notice that the only moment that it matters whether a resource is official or not is json serialization
    - Because it determines whether we serialize it to external_resoruces or not
    - So it's not a problem if during serialization we do some recalculations to determine if something's official or not
    - We need to allocate the needed official resources somewhere anyway
    
    - Why not just go the easiest route and have a separate pool of official resources outside of the editor_project struct?
        - Always allocate all official resources
            - note this still should not impact our performance
                - only images used on the scene will be loaded to memory
                    - later we'll optimize so only thumbs are loaded for the filesystem view or something
                - the created resource structs will be useful for indexing in the filesystem
        - the only nuisance is that we'll need a general dereference that takes into account both pools

- Instead of the whole fukcery with pools why not just identify every resource with a string?
    - pro: just std::unordered_maps
        - although once we have apis this doesn't change a lot
    - pro: solves the problem with identifying official ones
        - since we have the @
    - pro: 
    - con: have to use pseudo ids right away
    - con: have to recalculate every time we dereference instead of centralizing this
    - con: performance vs integer-based ids?
        - but when does it matter?
        - The only complexity that matters is json loading complexity for now
        - And for that we'll have to dereference those ids by strings anyway
        - Performance shouldn't matter all that lot
        - And it's about runtime data layout so this can always be improved later


- Alright, what about differentiating custom ids and official ids in binary?
    - For a unified api, we could allocate the used official resources in the pool
        - But hey, we have our own id structs, so we can easily pass a boolean there
        - This saves us a lot of pool manipulation operations
        - This simplifies data layouts too because we don't have to hold that "is official" data in resources themselves
    - So that an id refers to either official or not
    - But we have to determine which ones are used
        - Upon writeout, how do we detect that a resource is official and thus does not need to be saved?

    - In the cosmos, it does not matter whether a resource is official or not

- Co do resourcow 
    - i tak musisz hash walnac wiec nie ma po co sprawdzac czy jest default przy pisaniu
    - rescan
        - usuwamy te co nie sa 

- Dragging stuff into the scene
    - Docking breaks it because we basically have an imgui window sitting there
        - and it fetches all inputs probably
        - we have to make it somehow "no inputs"
        - or make a corner case for whether imgui input should be fetched
        - in any case we should be able to 
    - I wouldn't worry for now about background becoming invisible with particular dock configurations, it's a corner case
        - solved: ImGuiDockNodeFlags_NoDockingInCentralNode
    - Btw it's better with an empty central node because
        - Handles inputs properly out of the box
        - Won't unnecessarily occupy space with the single tab 
- However if we properly made the scene another window, we would right away solve problems as:
    - showing the "unsaved" mark next to the tab name
        - jakby sie uparl mozna nawet przy Project dac albo nazywac Project nazwa projektu
    - right away implementing mouse navigation with offset
    - I still think that could be done later

- let's implement the last opened project path for speed of iteration
- Haszy chyba na razie nie potrzebujemy, tylko do podpisywania

- Later also do some unit tests for json readwrite

- Let's leave it at "arena" nomenclature
    - Will even easier to find in code

- Open selected/Create from selected
    - Name your clone/Name your new arena

- "Choose a unique name for your map, be creative!
If someone has a map with the same name,
they would have to delete it in order to play yours."

- Reading the about section from the maps
    - for now let's just read them whole

- Creating new project

- co jak po drodze jest symlink? my tam robimy przeciez tylko weakly canonical
    - chyba ze weakly rozwiazuje symlinki

- editor_setup -> editor_setup
Nie nazywajmy tego arena editor tylko użyjmy najbardziej oczywistej nazwy

- Breaking changes that e.g. change signature verification scheme could be done by changing the builds/latest to e.g. builds/latest-2
    - In practice, builds/latest would always have the latest version so that it's easy to link it for people who don't have a binary yet
    - but the binaries would have defaults that point to symlinks like builds/latest-1, builds/latest-2 etc.
    - Not for now but good to have this planned

- masterserver -> serverlist maybe
- test code_escaped_nick against \`

- Balance datum, either way more expensive or less velocity
    - it's way more powerful than e.g. szturm

- A centered popup with the after-update message
    - The current right-bottom notice is hard to notice

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

- Sentience -> Character please

- Fix version numbering on MacOS
    - It's probably because of the shallow clone

- Left-handed option

- Fix those mag refills please


- Leaderboards shown in the main menu
- "Host a server" button right in the arena editor

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
