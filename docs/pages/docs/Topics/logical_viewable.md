---
title: Logical viewable
tags: [topics, flyweights]
hide_sidebar: true
permalink: logical_viewable
summary: | 
  If for a [logical asset](logical_asset), there exists a [viewable](viewable) from whom it can always be completely regenerated, it is called a **logical viewable**.
---

## Examples

A definition of a game sound, which is a *viewable*, contains a path to the sound file.  
However, the game logic needs no such information.  
It may need, however, to know the duration of the sound, to know when to delete the entity containing the corresponding [sound existence component](sound_existence_component).  
The distinction comes here:  
- A *viewable* may contain a path to the resource file.
- A **logical viewable** may contain something like the duration in seconds, or width and height in pixels. 

## Notes

One may be tempted to think of logical viewables as of another case of [inferred cache](inferred_cache).  
That is not quite the case, because logical viewables might be serialized to the disk so that an [intercosm](intercosm) is guaranteed to play correctly, regardless if some *viewables* change on the disk.  
For example, someone may replace an image or a sound on the disk, effectively giving it different dimensions or duration, but, as we have saved the original value of logical viewables,  
we may still use them, keeping the behaviour of the simulation identical, despite the viewables now looking different.  
Thus, logical viewables are indeed **inferred**, but they are not **caches**.  

