---
title: ToDo
hide_sidebar: true
permalink: todo
summary: Just a hidden scratchpad.
---

window should not be concerned with mouse pos pausing and last mouse pos.
let there be some separate mouse wrangler class or sth

fix freetype charsets

animation should work statelessly, in particular it should not set values to sprite.

pass display size and implement augs::get_display

we could make dynamically allocated tuple of overridden definitions for an entity that would normally just take one id,
that would map to a tuple of ids. as overriddes would be anyway rare that is will not hinder performance
we could always just say that we will always require new type to be defined wherever we would like to override something
so instead of explicitly making the thrown body bullet in the hand fuse logic, we'll just always leave it to the author to create a bullet body and assign a type id into the proper component


Types will have prefabs to be able to choose from.
E.g. a functional gun.
a functional sound effect.
a functional particle effect.
thus we won't hold particle effect inputs in components but just entity ids.

editor should print "types of selected entities" and their common properties, identically as with entities/components

editor bindings:
v - begin selection with arrows

