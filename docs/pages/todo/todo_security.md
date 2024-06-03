---
title: ToDo security
hide_sidebar: true
permalink: todo_security
summary: Just a hidden scratchpad.
---

- OAuth
	- likely use authorization code instead of implicit grant

- TODO: Check if the previous file transfer is complete, for security.
Should probably check if the channel is empty, and if it's not, just kick.

- prevent dangling users on ranked servers due to various edge conditions
	- e.g. can linger unauthenticated while downloading the map

- Think what happens when the server loads an incorrect arena or just fails to load it for whatever reason
	- currently a default one is loaded instead
	- shouldn't be a problem security wise but clients will desync if they happen to load the arena correctly

- watch out for unsafe_serialize: we should impose limits on container sizes when stuff is serialized just with read_bytes using introspection etc

- Watch out especially where servers pass arena name in their masterserver heartbeat
	- That "filename" could potentially lead to moving/removing a system folder if not sanitized!
		- Because we might request replacing an arena if we detect an existing one with the same name
	- Easy to sanitize though (literally just alphanumerics and underscore), we'd have to really screw up for this to be exploited 
		- But watch out

- Re-check const size containers if max sizes are respected when serializing json or other formats
	- esp. around convex shape partitions 
		- check for corner cases here still, the indexing is a bit complex here

- Any security risk in allowing any names for entities on the map?
	- Not sure, they would have to somehow be reported to outside servers but aren't really (except nicknames which are sanitized)

- Test if nicknames can't be exploited somehow
	- Either by maximum/minimum length, duplicate names logic, special characters etc.
	- Filter nicknames for all unicode whitespaces as well
		- Though the game won't even display them properly so probably no point
		- Maybe also do a parameter for printer to disallow newlines

- Still we need to test with 65k entities once we optimize editor and performance doesn't suck ass
	- But we can only do this after having the proper editor to not fuck up the old maps

- After disabling static allocations, we still have the theroetical limit of 65k entities, this could be breached but not anytime soon

- We should have a thorough audit of what happens when entity limits are exceeded
	- Flavours not so much as it is somewhat more controlled

- Re-check codec security before deploying a version with direct arena transfers
	- Consider allowing only several simple formats
		- jpg, png, gif, ogg
			- why even use wavs?
	- Esp. something like gifs? For which we'll need a separate version
	- How reputable is lodepng too?

- Validation of downloaded arenas
	- Enforce small miniature size when loading it in the project selector
		- or someone might upload a map with a nasty miniature size
		- we might just prevent from downloading it actually

- Protect masterserver from:
	- spoofing
		- just send a challenge packet
	- impersonating server heartbeats
		- actually requires encryption
	- perhaps let's just use yojimbo for all udp commands?

- Cooldowns for chat
	- Both client side and server side
	- Although maybe later, at least we'll ban griefers early

- Disallow massive forces for dropping transfers
	- Just trim against the throw mults of the capability
		- we can allow smaller, why not 

- If client net vars are out of bounds, disconnect
	- E.g. if it sets to never squash the entropies which could dangerously grow the client's entropies buffer

- Don't let the client spam with entropies too much?
	- Though they will be squashed anyway

- Tighten server->client security
	- Apart from being bandwidth-inefficient, current proof-of-concept net serialization performs no checks
		- that is also the case for client->server for now 

- Money persistence: for now someone can re-join warmup indefinitely and spawn arbitrary number of items from the shop,
overloading the server

## Done 

- Static allocation limits for entities and flavors
	- Lol these limits will eventually have to be lifted
		- Imagine a person with $20k, they can buy 40 mini knives
		- 40 of them is enough to clog the server, alright that's still quite a lot

	- Isn't static allocation bullshit anyway?
		- We shouldn't worry about it for now, we can set the limits sufficiently high for now
		- Technically it speeds up flavor dereferencing when we're iterating over all entities
			- But the fastest would be a pointer anyway
				- And we really should use pointers here because flavors are constant unlike entities
				- When we rewrite the solvables we won't have to rewrite the flavor pointers anyway 

	- Static allocation limits: we need to have a message about there not being enough space in the cosmos
		- This should prevent playtesting at all and should happen while we edit already
			- Actually should prevent the creation of any new node
		- This is mostly problematic for entity types later created in the game dynamically
			- Like weapons
			- So we should probably better limit firearms way before they hit the allocation limit
		- We should totally use the static allocation though to properly manage the posibility of too many entities
			- this will pay off in the future
		- Let's test this tomorrow
