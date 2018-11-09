---
title: Creating charges
tags: [editing] 
summary: |
    This article will guide you through the process of creating a completely new ammunition type inside the [Editor setup](editor_setup).
permalink: creating_charges
---

## Steps

{% include flavour_creation_tip.html %}

You will need three flavours:

- The flavour of the cartridge itself - basically, just an item that can be dropped to the ground or stored in a backpack.
	- You will need an image.
	- Examples:
		- Cyan charge: {% include official_img.html file="cyan_charge.png" %}
		- Orange charge: {% include official_img.html file="orange_charge.png" %}

- The flavour of the round that is spawned upon shooting.
	- You will need an image.
	- Examples:
		- Cyan round (colorized in the game - the image is reused for rounds of different colors): {% include official_img.html file="round_trace.png" %}
		- Orange round: {% include official_img.html file="orange_round.png" %}
			- Almost identical to the charge, but sometimes it may be appropriate.
	- It is recommended to give it a strong neon map so the round is visible in the dark.

- The flavour of the shell that is spawned upon shooting.
	- You will need an image.
	- Examples:
		- Cyan shell: {% include official_img.html file="cyan_shell.png" %}
		- Orange shell: {% include official_img.html file="orange_shell.png" %}

