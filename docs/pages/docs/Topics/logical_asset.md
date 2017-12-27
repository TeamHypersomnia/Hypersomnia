---
title: Logical asset
tags: [topics, flyweights]
hide_sidebar: true
permalink: logical_asset
summary: | 
  A **logical asset** is an object that, given a particular [viewable](viewable), holds only that part of whose knowledge is necessary for [solvers](solver) (the game *logic*).  
  It is always completely regenerable from the given viewable.
---

## Notes

One may be tempted to think of logical assets as of another case of [inferred cache](inferred_cache).  
That is not quite the case, because logical assets might be serialized to the disk so that an [intercosm](intercosm) is guaranteed to play correctly, regardless if some viewables change on the disk.  
For example, someone may replace an image or a sound on the disk, effectively giving it different dimensions or duration, but, as we have saved the original value of logical assets,  
we may still use them, keeping the behaviour of the simulation identical, despite the viewables now looking different.  
Thus, logical assets are indeed **inferred**, but they are not **caches**.  

## Examples

A definition of a game sound, which is a *viewable*, contains a path to the sound file.  
However, the game logic needs no such information.  
It may need, however, to know the duration of the sound, to know when to delete the entity containing the corresponding [sound existence component](sound_existence_component).  
The distinction comes here:  
- A *viewable* may contain a path to the resource file.
- A **logical asset** may contain something like the duration in seconds, or width and height in pixels. 

## Further work
<!-- TODO -->

Animation, recoil player and physical material currently have no "viewable" counterpart, but they are nevertheless logical assets in the code.  
Under the current definitions, they should be part of [entity types](entity_type), but maybe it makes sense to share animations and materials?  
It, however, makes little sense to make a physical material a viewable as it may very well influence state of the simulation (sounds etc.).
The recoil player could very well be a part of gun definition.

We might change the definition of a logical asset not to be tied to a viewable whatsoever; only that it is a resource not tied to any entity type.
