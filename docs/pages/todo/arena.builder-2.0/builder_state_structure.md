# Separate proxy structures for all (or almost all) flavours

- Do we want a simplified state structures for all components too?

# Many component properties will become flavourized

- We'll be able to remove "colorize" and "colorize_neon" from sprite component because the flavours will be created automatically
	- Actually most of sprite component will be able to be eliminated
	- Might be true of other components too
	- Remember that as we have less and less in components will make logic faster and it is the bottleneck
		- rendering can be slower, no matter
	- Yeah, the discrepancy between what we decide to actually put into a component (because it would create so many flavours)...
		- Is the precise reason we'd like to have editor-dedicated structs for serialization
	- BTW even light component could be all flavourized
	- All of this will seriously decrease the amount of stuff we have to hold in the solvable which is frigging GREAT

# Intercosm as a cache

- All flavours, all solvables, all viewabels defs, all logical assets...
	- Actually the rulesets could be compiled this way too
- Which is super cool because it will be easy and quick to read it from disk.
	- Though these separate files never occur in the code without each other, still it's easier to load it in particular structures of the cosmos
	- So perhaps let's leave these files at that
- actually let's make a single file for speed of reading
	- ez pz
- And write the timestamp after writing that file
	- to always regenerate in case of any doubts about authenticity
	- the same timestamp should be written next to the project file on writeout?
		- and it's always written as the last thing after all resource files are written too
	- keep version stamps too
