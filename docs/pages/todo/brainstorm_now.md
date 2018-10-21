
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- Stop the whoosh sound on obstacle hit
- Apply shake and impact of the damage, always mult cooldown by two

- Melee combat
	- melee_fighter component
		- so that we don't have to hold this state per each melee weapon
	- Returning animation
		- Don't. Simply specify a target rotation to which the character is to be rotated
			- We might even set a slower rotating speed for a while
		- If we hit a solid obstacle, we always return with the animation reversed
		- Do we want another animation for returning when the hit was complete?
			- If we don't want it, we can always set the same animation
	- communicating animation offsets
		- We need information from the animation itself
			- Actually we do hold animations in logical assets so we are all set
	- A melee attack cannot be interrupted, except when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing

- Flipping of editor selection

- Easily spawn loaded weapons or magazines
	- For now, let instantation load them all by default

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

- Context help
	- Enums corresponding to text files
	- Problem: arbitrarily colorized text is not supported in imgui
