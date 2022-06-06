# General: Interface

- Do we open a separate tab for the resources? Or do we use the inspector?
	- I guess a tab is in order
	- alright, godot actually shows it in the inspector, perhaps it is more intuitive
		- But it actually shows a separate window for scripts. So maybe if we have a lot of properties for images, let's have them edited in a separate window

- So let's open the as a new tab for now
	- It nicely fits into file-per-tab scheme too
	- Will need its own saving logic

# Animation

- A resource
- Usually next to the frames in the folder
- A preamble with paths
- later use ids

# Images

- We need to be able to edit neon map colors conveniently
	- We can open a one-shot floating window

- The legacy editor will serve for skinning new character sprites
	- It's a developer tool after all

# Identification

- GUID-based approach makes a lot more sense than path-based
	- See https://github.com/godotengine/godot/issues/15673#issuecomment-357455291
	- We'll just re-scan the filesystem every time we focus the window right? Did we do it like this earlier?
