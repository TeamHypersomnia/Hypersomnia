---
title: Creating charges
tags: [editing] 
summary: |
    This article will guide you through the process of creating a completely new ammunition type inside the [Editor setup](editor_setup).
permalink: creating_charges
---

ToDo: instead of creating detailed guide for this, we should simply introduce a creator window that only shows the most relevant fields for editing.

## Steps

{% include flavour_creation_tip.html %}

All images mentioned here should be selected inside Sprite invariant->Image id in the respective flavours.  

You will need these flavours:

- The ``shootable_charge`` flavour of the cartridge itself - basically, just an item that can be dropped to the ground or stored in a backpack.
	- You will need an image.
		- Examples:
			- Cyan charge: {% include official_img.html file="cyan_charge.png" %}
			- Orange charge: {% include official_img.html file="orange_charge.png" %}
	- Make sure to customize:
		- item invariant's ``space_occupied_per_charge`` as it determines how many charges fit into respective magazines. 
		- cartridge invariant's ``shell_trace_particles`` to give a unique effect to the hot shell that has just been cast out of the chamber.

- The ``plain_missile`` flavour of the round that is spawned upon shooting.
	- You will need an image.
		- Examples:
			- Cyan round (colorized in the game - the image is reused for rounds of different colors): {% include official_img.html file="round_trace.png" %}
			- Orange round: {% include official_img.html file="orange_round.png" %}
				- Almost identical to the charge image, but sometimes it may be appropriate.
		- It is recommended to give it a strong neon map so the round is visible in the dark.
		- It is recommended to give a proper physical shape to the bullet in the image meta.
			- The shapes for round images should remain convex to avoid some strange physics bugs.
	- Make sure to customize:
		- missile invariant
			- ``pe_damage_ratio``. This ratio determines the amount of Personal Electricity that is required to spawn this round in a magical weapon, like Amplifier arm. A reasonable default is ``0.25``.
				- For example, since the ``ELECTRIC_MISSILE`` round flavour has a base damage of ``42``, the PE required to spawn it is ``floor(42 * 0.25) == floor(10.5) == 10``
			- ``trace_particles``. This determines the stream of particles that flies along the bullet in mid-air.
				- Also, ``trace_particles_fly_backwards``
			- ``trace_sound``. This determines the sound that the bullet will make when it passes close to your ear.
			- ``muzzle_leave_particles``. This determines the explosion that happens during a shot.
			- The entire ``damage`` field. What it does is self-explanatory.
			- ``ricochet_sound`` and ``ricochet_particles``. Self-explanatory.
			- ``max_lifetime_ms``. Determines how long will the bullet live, in milliseconds.
			- ``recoil_multiplier`` - The contribution of recoil per every shot. ``1`` means no contribution.
			- ``remnant_flavours``. A vector of flavours that will be 
			- ``homing_towards_hostile_strength`` - The round can "magically" seek the nearby enemies while in mid-air.
				- For example, the Amplifier arm's missile flavour has this force set to ``1.0``.
			- ``damage_falloff_starting_distance`` - Not implemented.
			- ``minimum_amount_after_falloff`` - Not implemented.
		- Optional: rigid body invariant
			- If you want the missile's velocity to not be linear, but to start fast and damp over time, 
			  you will need to set a non-zero **linear damping**. For example, the standard shotgun pellets have a damping of ``3``,  
			  whereas all other rounds have it ``0``.

- The ``remnant_body`` flavour of the shell that is spawned upon shooting.
	- You will need an image.
		- Examples:
			- Cyan shell: {% include official_img.html file="cyan_shell.png" %}
			- Orange shell: {% include official_img.html file="orange_shell.png" %}
	- Make sure to customize:
		- remnant invariant.

- The ``finishing_trace`` flavour of the entity that is spawned upon the bullet's impact.
	- Usually, you should just set the Sprite invariant to be the same as of the round itself.
		- For example, finishing trace of the cyan round will smoothly shrink with the same sprite, until it disappears completely.
	- Later, this flavour might contain some explosion information for exploding rounds.

- Optional: One or more ``remnant_body`` flavours of the bullet remnant. These will be spawned upon the bullet's impact.
	- You will need an image.
		- Examples:
			- Steel round remnant 1: {% include official_img.html file="steel_round_remnant_1.png" %}
			- Steel round remnant 2: {% include official_img.html file="steel_round_remnant_2.png" %}
			- Steel round remnant 3: {% include official_img.html file="steel_round_remnant_3.png" %}
	- Make sure to customize:
		- remnant invariant
			- ``lifetime_secs``. Obviously, to avoid bloating the scene, the bullet remnants have a very limited lifespan. 
			  This value shouldn't be greater than a mere second or two.
			- ``start_shrinking_when_remaining_ms``. Self-explanatory.
			- ``trace_particles``. This determines the stream of particles that flies along the remnant in mid-air.
	- Remember to set the ``remnant_flavours`` in the missile invariant of the flavour that is to spawn these remnants upon shooting.

If the ammo is intended to be used for a shotgun with a chamber magazine,  
and thus each cartridge is going to be loaded by hand, you'll need to set a proper price in the [item invariant](item_component).
