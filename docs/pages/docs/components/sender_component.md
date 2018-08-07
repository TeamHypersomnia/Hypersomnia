---
title: Sender component
tags: [components]
hide_sidebar: true
permalink: sender_component
summary: A sender component is always present with dangerous objects (e.g. armed explosives, flying bullets) as it tracks which entity (e.g. a character) is "guilty" of their coming to existence.
---

Fields:
- ``direct_sender``
	- In case of a flying bullet, the gun with which it was shot.
		- e.g. to know what was the item with which one was killed.
- ``capability_of_sender``
	- The character who held ``direct_sender``.
		- e.g. to know who has kiled who.
- ``faction_of_sender``
	- The faction of ``capability_of_sender``.
		- e.g. to disallow Resistance from defusing its own bombs.
- ``vehicle_driven_by_capability``
	- The vehicle which was driven by ``capability_of_sender``.
		- We are only interested in the vehicle driven by capability at the time of sending.
		- e.g. to disable collision between the vehicle and the bullets shot by the driver.
		
In case of a released explosive (e.g. a planted bomb or a thrown grenade), both the direct sender and its capability  
point to the character who has thrown it.  

In case of explosion bodies generated from a bomb, they all get their sender components copied from the bomb.  

An explosion, on detonation, sends damage messages.  
TODO:  
The damage messages should also convey the information about the sender, so that we know who's killed who and with what.
