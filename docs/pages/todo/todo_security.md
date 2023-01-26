---
title: ToDo security
hide_sidebar: true
permalink: todo_security
summary: Just a hidden scratchpad.
---

- Still we need to test with 65k entities once we optimize editor and performance doesn't suck ass
	- But we can only do this after having the proper editor to not fuck up the old maps

- After disabling static allocations, we still have the theroetical limit of 65k entities, this could be breached but not anytime soon

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
- We should have a thorough audit of what happens when entity limits are exceeded
	- Flavours not so much as it is somewhat more controlled

- Re-check codec security before deploying a version with direct arena transfers
	- Consider allowing only several simple formats
		- jpg, png, gif, ogg
			- why even use wavs?
	- Esp. something like gifs? For which we'll need a separate version
	- How reputable is lodepng too?

- Test if nicknames can't be exploited somehow
	- Either by maximum/minimum length, duplicate names logic, special characters etc.

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
