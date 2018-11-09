---
title: Creating guns
tags: [editing] 
summary: |
    This article will guide you through the process of creating new guns inside the [Editor setup](editor_setup).
permalink: creating_guns
---

## Prerequisites

### Preparing the ammunition

It is fine to reuse an existing cartridge flavour for your new gun.  
For example, both BILMER2000 and Datum Gun shoot cyan charges for their cartridges,  
simply because they are aesthetically compatible.  

If you wish to create a unique ammunition type for your weapon, follow the [gun charge creation guide](creating_charges).

### A magazine

You should never reuse a magazine flavour that is named after a weapon with which it is compatible.  
Thus, most of the time, you will create your own magazine flavour.

You will only need a magazine sprite.

Make sure to set a balanced price for the magazine within the [item invariant](item_component).

### A recoil pattern

Most of the time, you will use the Generic recoil pattern for the gun.
*Actually, it is not yet possible to create new recoils in the editor...*

## Creating the gun

It takes a lot of assets to create a unique experience of a gun.  
While you can perfectly reuse the official files whose names begin with "standard_" preffix,  
it is really worthwhile to manually create a complete collection of resources for each and every gun flavour.

A gun will need:

### Graphics

- The gun's sprite.
- Optional: the gun's shooting animation.
	- Can have any number of frames.
- The gun's magazine sprite.

### Sounds

## Final touches

Always test how your gun shoots.  
Make sure to set a balanced price for the magazine within the [item invariant](item_component).
