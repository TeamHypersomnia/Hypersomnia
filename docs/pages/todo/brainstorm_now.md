---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- adjust doppler factors of bullet trace sounds

- Add inertia for the target rotation when we hit an obstacle
	- Copy the lerp code from interpolation as it was mostly correct

- Checks for animation bounds in melee system

- Chosen solution: let direct attachment offset calculate the knife position so that the entity is always positioned where it is seen.
	- Additionally, manually query once every time the animation frame changes.
		- The change in animation frame will properly be detected by the melee system.

- Use gift wrapping algorithm for calculating colliders
	- we can quickly implement this on our own

- Setting fixtures vs querying knife fixtures on each step
	- entity-based approach to positioning
		- Then the direct attachment offset must calculate the offsets, I guess.
		- Problem: The hit trigger won't exactly correspond to the knife's shape as we will take convex hulls
		- pro: we can still base hits on a significant constant size vector
		- pro: don't have to fool around with triggers, just accept begin contact events
		- con: there might be some physics glitches since engine will try to push away the collider from the knife fixture
			- or actually it might be even more realistic
		- con: we have to reinfer the melee's colliders every time an animation frame changes.
			but only the melee's colliders, not the physical body.
	- query on each step of in_action
		- pro: finer control
			- over what exactly? Probably only the crosshair.
				- but we can implement a clause for this
					- OR EVEN FREEZE ROTATION FOR THE DURATION OF THE HIT!
						- that's right! that will also prevent cheats where we make a full rotation just to hit something.
		- con: trace-particles won't work out of the box
		- con: trace-sounds won't work out of the box
			- this is big
		- con: knife is always hittable where it was initially standing, instead of where it is in the screen
			- actually this makes our clashes quite complex because we'd have to perform manually n^2 search

- Problem: If we simply reset fighter state on switching weapons, it might be used to shorten cooldowns
	- **Solution**: set_cooldown sets a max of current cooldown and the currently held weapon's cooldown
		- always set it during item manipulation
	- If we leave state as it is, on returning to a melee weapon we can still see remnants of the returning animation
		- This isn't much of a problem

- Always pre-set the cooldown values **and don't use the maximal values of the held weapon** during processing

- And still process cooldowns even if we don't have the conditions for a melee attack in the first place
	- So we're not getting glitches later

- Remember to reset input flags of the character itself on performing transfer
- Melee filter: flying bullet

- Stop the whoosh sound on obstacle hit

- On clash, apply shake and impact of the attack's damage info, always set to twice the cooldown
	- Though we need to play returned animations

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
	- A melee attack cannot be interrupted, except for solid obstacles and when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing

- Easily spawn loaded weapons or magazines
	- For now, let instantiation load them all by default

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- Game events log and chat
	- In the same window
	- ImGui or own GUI?
		- We actually have some textbox code we can introduce later for chatting
			- Better control over such an important feature
			
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
