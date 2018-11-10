---
title: Creating guns
tags: [editing] 
summary: |
    This article will guide you through the process of creating new guns inside the [Editor setup](editor_setup).
permalink: creating_guns
---

{% include flavour_creation_tip.html %}

ToDo: instead of creating detailed guide for this, we should simply introduce a creator window that only shows the most relevant fields for editing.

## Prerequisites

### A magazine

For your new gun, you should never reuse a magazine flavour that is named after a weapon with which it is compatible.  
Thus, most of the time, you will create your own magazine flavour.

For this, you will only need an image for the magazine sprite.
- To edit how the magazine fits into the gun, you should edit two image offsets:
	- Firstly, the gun's ``detachable_magazine`` offset.
	- Secondly, if you'll allow more than one magazine type for a gun (not recommended now), there is an ``attachment_offset`` in each item.
		- Note that an item will only ever fit into a single type of attachment slot, so it makes sense to have such a generic name for this offset.

Make sure to set a balanced price for the magazine within the [item invariant](item_component).

#### Preparing the ammunition

It is fine to reuse an existing cartridge flavour for your new magazine type.  
For example, both BILMER2000 and Datum Gun use the Cyan charge flavour for their cartridges,  
simply because they are aesthetically compatible.  

If you wish to create a unique ammunition type for your weapon, follow the [gun charge creation guide](creating_charges).

#### Setting the compatible ammunition type

Navigate to the magazine's [container invariant](container_component), choose Item deposit,  
and set ``only_allow_flavour`` to your cartridge flavour of choice.

Tweak ``space_available`` to determine how many charges will fit inside.
You can change the amount of space occupied

### A recoil pattern

Most of the time, you will use the Generic recoil pattern for the gun.
*Actually, it is not yet possible to create new recoils in the editor...*

## Creating the gun

It takes a lot of assets to create a unique experience of a gun.  
While you can perfectly reuse the official files whose names begin with "standard_" preffix,  
it is really worthwhile to manually create a complete collection of resources for each and every gun flavour.

A gun will need:

### Graphics

- An image for the gun's Sprite invariant.
	- Viewed:
		- When the gun is idle,
		- in the knockout indicators in the top left corner,
		- and also in inventory GUI.
	- You should edit the ``gun_offsets`` of the chosen gun image to set some important metrics for the gun.
	  See Images GUI (Alt+I).
- Optional: the gun's shooting animation.
	- Can have any number of frames.
- The gun's magazine sprite.

### Sounds

### Customization 

- Gun invariant
	- ``adversarial.knockout_award`` - the amount of money that a player gets for killing with this weapon.
	- ``magic_missile_flavour`` - If the gun is to be based on Personal Electricity rather than material ammunition, this is the flavour of the missile that will be spawned per each shot.
		- If this flavour is set, the weapon will never use its chamber or magazine slots for gathering ammunition.

## Final touches

Always test how your gun shoots.  
Make sure to set a balanced price for the magazine within the [item invariant](item_component).
