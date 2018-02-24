---
title: ToDo now
hide_sidebar: true
permalink: todo_now
summary: Just a hidden scratchpad.
---

## Microplanned implementation order

- Look through all todos before you take on editor
- "unique" naming for unique sprite decorations et cetera
- complete editor setup might come in handy for debugging
- Audiovisual caches should always check if the transform exist because sometimes the transform might be lost even due to having been put into a backpack.
	- especially the interpolation system.
		- **it should crash on transfer to backpack when we correct the transfers **
	- just use find transform
	- use clear dead entities as well because it minimizes sampling errors if e.g. the solve wouldn't run between steps
		- though I think we always run audiovisual advance after step

- Local setup should record session live
	- This is really important in the face of bugs.
	- Or just get rid of test scene setup for now and let it by default launch a scene in editor that records inputs

- For continuous sounds, sound system should probably assume the same strategy as an inferred cache.
