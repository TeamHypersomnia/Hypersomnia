# Migrating existing maps

- Honestly, we should get ready to make these three maps from scratch again.
	- We can pay somebody to do it, and it will be the ultimate test of the new editor.
	- Writing conversion code will be a LOT more work.
	- It will be the ultimate test for the new system.
	- The conversion code will NOT create a correctly prefabbed map anyway - we have to do it by hand
	- We can just export the important layout somehow

# Ad-hoc fixes

- We should move all gfx/sfx of official maps to actual official content
	- And simply assign it to properly named biome folders
	- Some complicated prefabs can already be map-specific
	- This way we won't copy the files when someone wants to make a variation out of a template

- Instead of having delete_laying_on..
	- Okay, no, that might be useful to delete weapons for some rulesets (e.g. gentlemanly mode)
	- Actually... perhaps a node reference to disable would be a better option
		- e.g. then we can disable only AWPs on a map like fy_snow

- Keep the variable names really short for the inspector to look good
- View->Window

- Refresh imgui font when changing size in settings

# Legacy editor considerations

- Compile out the legacy editor from production.
	- It will only be a developer tool after all and will massively speed up compilation times.
	- It will also reduce binary size.

- We'll keep the old editor code both as a reference and as a legacy world inspector
	- The old editor will still be useful as the inspection tool for translated worlds
	- The old editor code to keep won't be an entire setup, but rather only specific windows for flavour/entity editing

- User won't be concerned with flavours
	- We'll just translate the state of all prefab instances into the required flavors
		- We'll thus essentially implement COWs
	- actually most of properties will only concern flavours
		- we'll probably code GUI for all properties in Builder by hand
