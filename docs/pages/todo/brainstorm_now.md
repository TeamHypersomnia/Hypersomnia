---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Melee combat
	- melee_fighter component
		- so that we don't have to hold this state per each melee weapon
	- Primary and secondary attacks for knives
		- Akimbo is thus handicapped only to the primary
			- But really it makes little sense to carry two knives
	- A melee attack cannot be interrupted, except when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- Game events log and chat
	- In the same window
	- ImGui or own GUI?
		- We actually have some textbox code we can introduce later for chatting
			- Better control over such an important feature

- Windows back-port
	- use non-multisampling fb configs on Windows
		- proven to improve performance twofold, on linux

- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic
	- Seed will have to be sent in the beginning anyway, along with the state
	- Some amount of initial information will need to be transmitted anyway
		- Like current players?
		- Isn't this all a matter of sending the bomb mode state?

- Probably somehow disallow arbitrary inferring of relational cache?
	- There was some unresolved crash problem with this.

- check in editor if the saving/opening path is a valid folder?

- make reveal in explorer work for both files and folders
	- cause it also works for dirs

- Context help
	- Enums corresponding to text files
	- Problem: arbitrarily colorized text is not supported in imgui
