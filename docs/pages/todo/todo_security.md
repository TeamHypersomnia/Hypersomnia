---
title: ToDo security
hide_sidebar: true
permalink: todo_security
summary: Just a hidden scratchpad.
---

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
