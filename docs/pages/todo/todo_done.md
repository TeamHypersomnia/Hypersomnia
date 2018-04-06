

### Microplanned implementation order (done):  

- Should rigid body synchronizer check whether the cache is constructed or always assume that it is?
	- **We might later implement it so that a body might be absent under certain circumstances to save memory.**
		- A single check does not hurt so we'll leave it as it is.

- rename fundamental to always_present
	- so that it is intuitive that there are no callbacks for adding and removing

Reiteration #1:

- Getting information about success from reinference
Some configurations of solvable significant do not allow for an inferred counterpart that is consistent.  
E.g. if vector of items inside a container is inferred, and one would like to perform a transfer by 
Here's where helper functions might be necessary. A perform_transfer function should only do as much as predict the outcome of reinferring the item component with newly set current slot,
and if it is successful, do set it and reinfer the item.

we consider whole type overrides too complex architeciturally:
- the assumptions about inferred state for entity flavours are gone; previously it was just generated once and set in stone, always correct
- noise while getting those invariants
- we should discourage solver from ever modifying that state
	- otherwise to look good we would need to always completely destroy that entity and recreate it with components with default associated values

- fix trace interpolation problems (possibly others) in release
	- observation: debug works
		- divergence is consistent after clearing the build folder
	- numerical problem perhaps?
		- "looks like" the interpolation cache may be getting reset too often
	- fixed: a temporary for the version was invalidated

- Currently, logic transform might be ambiguous if:
	- The entity has a rigid body and also fixtures that belong to some remote rigid body.
		- We will always prefer fixtures if they exist.

- If we want to destroy b2Body data once all fixtures are destroyed, it would be a case without precedent where the state of the parent cache depends on some of its children.
	- Let's just allow bodies to stay without fixtures for now as it will be easier. We'll see what happens.
	- Thus the former owner body won't have to be reinferred once a remote fixture entity gets destroyed.
	- In any case, we could just implement it in the physics engine side that the body gets removed from the world list if the last fixture was removed.

- Get logic transform will always acquire values using caches for speed.
	- **If it doesnt destroy performance, we can check if cache exists and fall back to value in the component.**
	- But such a thing would anyway probably be needed only in constructor?
		- not if we allow b2body to be destroyed
	- For now let's just leave it as it is so that we hit assert when we acquire logic transform at a suspicious time

- Invariantizing rigid body
	- velocity and transform should always be able to be set directly as they are temporary parameters of the simulation, not statelessly calculated on any state.
		- better wording: these variables are accumulators

- Remove owner_body from fixtures component and create a component named "custom rigid body owner" that overrides anything else

- change path of #include "game/messages/message.h"

- ditch stream displacement for a while
- remove completely the particles existence and sound existence component
	- logic will post relevant messages to create bursts
		- fire and forget bursts:
			- no owner assigned, just starting world transform
			- separate vector in particles simulation system
		- stream can be created in the name of an entity/offset
			- don't use purpose, just differentiate by orbital transforms
			- car will be able to stop engine's stream by looking up its orbital transform
				- bytewise comparisons should work after just assigning			
		- in which case the stream will be reset to the new value instead of creating additional stream
			- removes the need to hold child entity ids in sentience	
		- burst can be stopped in the name of an entity
			- used by the car
		- continuous streams could be queried statelessly by the simulation system

- should find_logic_transform return nullopt if entity is dead?
	- perhaps, for simplicity. we don't gain much by ensuring there

- Implement entity flavours incrementally.
	- We do it now to not repeat the code when we'll be writing improvements to inferences 
		- gun component 
			- magic missile def is just a child entity to be cloned around as needed.
				- thus it only becomes type id.
		- catridge component
			- whole component should become invariants and its child entity id fields should just become type ids
	- Get rid of component adders and deleters.
		- That will be only possible once we invariantize practically everything in the test scenes.
		- We'll thus need to make sender and others "always present".
- Current slot should stay in the item component as other fields of item will anyway be a private

- **Chosen approach for cloning and child entities**:
	- **Let there never be a need from a child to access the parent**.
		- Thus letting us eradicate the existential_child component.
		- E.g. instead of crosshair having an entity_id to its parent, let's handle the motions while knowing about sentience itself, without needlessly contextualizing.
		- E.g. for aabb highlighter, we can always just query the whole body if the entity is fixtural.
			- there would anyway be no point in otherwise expanding the aabb.
		- And if there is, let's have a cache.
	- Let us use the storing of child_entity_ids approach.
		- Pro: Cache-less calculation of children.
		- This lets us conveniently setup the clones of entities even with children entities.
		- Existential child becomes redundant. 
	- Let child_entity_ids be either hidden from the author (by constexpr) or just plain immutable.
		- can't have consts in components, though.
		- they will never change either way, but it's good to be sure.
	- In any case, whether we need the cache or we only assume that entities marked via child_entity_ids will be deleted, 
	it would be good and safe to have a synchronizer for each case where we specify an existential child. But we don't have time to play it safe.

- Cloning entities
	- We no longer need cloning for charged items because it is highly unlikely they would have a child entity 
	- An author might want to copy/duplicate something on their screen
		- Even character would need their crosshair child
	- There might be a cosmos inconsistent error thrown if author clones something
		- well then the editor may controllably catch it and revert
		- and in the solver we will take proper precautions

- Consider: what would it gain us to have e.g. cosmos solvable templatized completely upon an entity type,
 vs having all its members per and functions work manually upon all types?
	- in the same way with inferred caches since that would be implied?
	- at some point we must anyway wrap ops for all entity types and at the lowest level we do the better


- Thoughts about entity types
	- Storing flavuor ids and retrieving information
		- Might be useful for a flavour id to assume that some components and/or invariants are in existence
		- Then we would need to disallow setting incompatible types 
		- Cosmos could return std::optional<constrained_flavour_id<invariants...>> for a generic flavour id 
			- these will exist usually in definitions only
	- specifying types
		- tuples/trivially copyable tuples, because they will be easy to introspect and reason about
		- should components be added automatically?
			- we might want to arrange some components for better cache coherency
			- instead, let's just have "assert_component" instead of "implied_component"
		- proposition: a struct [[type]] and "using variants" "using components" inside
			- provides a canonical, consistent way to refer to an entity type everywhere
			- can also easily forward declare such types
			- maybe even enables us to define some custom hacks for the types in the structs?
		- proposition: using [[type]]_variants and using [[type]]_components
			- con: doesn't actually give a simple way to associate these two
	- first implement allocation and identification, processing can for now stay as it is
	- mixin templates won't make sense to be instantiated
		- because of the typed handles
	- typed vs general entity id and handles
		- Entity ids will need to have information about which type the entity is
			- A typed id that can accept two or more entity types still needs a number and is inperformant in the same way that a general handle is(probably)
				- One-typed ids will be very rare anyway
			- type id will be in cache once we access the entity id so it will be only several more cycles to calculate the offset
		- Sytematic processors (cosmos.for_each(...))
			- would acquire a type list of (possibly const-qualified) components and invariants 
				- the list specifies which ones we **get** for sure
				- we don't specify which ones we will try to "find"
				- process all entity types except those that don't have specified components for **get**
			- removing the need for processing lists
		- Thus let's just go with the general ones
		- Only the systematic processors should acquire typed handles via generic lambda callbacks
			- The interface should be compatible with entity handle except get should fail at compile time for non-existent invariants/components
<!--
		- It would be interesting to, for example, typize entity ids 
			- so that only a certain subset of entity types can be referenced by them
			- then we could know in compile time if get<some component> cannot be performed on a handle with a given id
		- Could be introduced at a later time and for now we could just introduce general entity handles and ids everywhere.
-->
	- The authors should not be concerned with customizing invariant sets but with choosing native, well-understood presets.
	- This could be even introduced incrementally, e.g. entity handle would still perform type erasure to not compile everything in headers.
		- For now the authors may also see just presets without being able to add or remove invariants
		- Specialized handles would be returned by for_each for processing_subjects in the cosmos
	- We can make the entity_flavour's populators in ingredients just templated functions
		- For the sake of not repeating test scene flavour logic for some different native types
	- We will totally get rid of processing component and calculate statelessly which that which needs to be calculated.
		- We anyway very rarely ever disabled something in processing subjects and we must always account for the worst case.
	- Refer to typed_entity_handle for draft.
	- Storage (TRANSPARENT)
		- Array of structs vs struct of arrays
			- Storage will be transparent to the logic, even if we don't introduce native types.
				- typed_entity_handle<character> will have perhaps cosmos& and character* 
				- general entity handle will have more than now, as it will have to perform type erasure. It will have type id and void* that will be reinterpret-cast. 
			- Changing ingredients to native types won't require more work than just adding a type specifier to get test scene type. 
				- meta.set will still apply as always
				- enums will also apply because many entity flavours might share the same native type
			- So we don't have to do it now.
			- We will specify storage for native types in tuples, thus we will be able to change SoA to AoS and back with just one compilation flag. 

- **moving disabled processing lists out of the significant state**
	- for now ditch the setter of rotation copying when switching driver ownership
	- most stateless, thence easiest is when we just easily determine when not to process an entity without relying on having proper fields set in processing lists
		- we should then only ever optimize when it is needed
	- or it can be part of inferred state which will complicate things a little
- fix step_and_set_new_transforms to iterate over entities instead
- make static asserts for presence of respective components and invariants, because it turns out that sometimes code may compile with one missing
	- as was the case with missing item invariant
- fix possible crash in pool due to fabricated ids?

- test travis build at introspector-generator
	- with matrix and specifying build names for better naming?
		- or just add custom names

- fix absolute or local
	- really, because we're even getting a crash


- Constructing entities
	- Solver might want to set some initial component values before inference occurs
		- Should be done in a lambda where a typed entity handle is given 
		- After which cosmic::create_entity will simply infer all caches
		- provide an overload which sets transform
			- static assert if the entity cannot set a transform
	- Most constructions and clones will request general flavours and thus return general handles
		- Thus let's for now expose just those general ones

- Fix bad transforms issue when dropping items

- fix disappearing wandering pixels?
	- we weren't initializing them properly

- Fix open current directory for editor in linux

- what if there is just "unsaved" file?
	- we load as if there were a real file
	- but we're emitting a warning

- rename local_test_subject to local_test_subject
	- even choreographic data will have their own viewers specified
	- useful to always have one

- timestamp all logs for convenience
- delete untitled folder on saving

- should inferred caches map from both guids and from ids?
	- no point. some indirection would be necessary anyway
	- just provide overload for getters that have a cosmos at hand

- you might want to write tests for pool, especially now that we have undo_free
- PROBLEM: guids as signi entity ids might:
	- hinder processing performance
	- increase code complexity
- on the other hand, plain entity_ids avoid these
	- **The overhead of pool metadata is really insignificant, even during frequent cloning.**
	- entity count * 4 shorts. So literally 2 ints per entity.
		- more is added with addition of a single component.
	- thus let's just implement undo_freeing in the pool
- the problem with entity ids in common signi is the same as with guids in common signi
	- won't be solved until after we introduce groups + templated components
- PROBLEM: simulating undo_delete_entitys in the pool is actually hard
- PROBLEM: if we're not storing whole pools and undoing/redoing with allocates/frees, we're anyway violating determinism
	- because order of entities in the pool will be different completely 
	- unless we properly simulate those undo_frees
		- plan
			- we store the real id where the deletion happened
			- on undo_free, move the element at real index to past the last element 
			- slot needs moving as well
			- write undo_delete_entityd content to the gap
			- decrease version

- (DISREGARDED) Important: make components contain entity guids instead of entity ids
	- And let the cosmos only serialize entities and not pools
	- that is because then we won't have to serialize entire pools
	- and operations like undeleting entities will be a lot easier because we won't have to simulate an old entity id
	- signi_entity_id
	- in case of groups, we're not doing something twice, because we'd anyway need to narrow down those ids
	- let entity_id be said about as "a temporary id", could later even become something with a pointer cache

- (DISREGARDED) should inferred caches map from guids or from ids?
	- should perhaps be both
	- fastest access
	- will have to templatize cache finders
	- most of the time, retrievals will happen

- resuscitate quick search because there's some useful highlight logic 

- clear selections on history changes!
	- really fix them

- rectangular selection could crash while undoing/redoing
	- why not calculate it statelessly?
	- just always finish the rectangular selection before redoing/undoing

- add entity names for descriptions
- add history gui
	- some simple description
	- perhaps timestamps?
- refactor go to all

- text_color and relevant text_disabled
- simple commands for filling

- fix std::array<internal_glyph, glyph_count> glyphs;

- implement unicode code points in printer/drafter

- Address ids are irrelevant to us because we will compare against a well-formed property id.
<!--
- when in editor mode, the address will always stay the same during dragging
	- because we will disallow any other action
- in gameplay mode, the address may suddenly change
-->
- rename minmax to "bound"
- use range widgets for minmaxes, with proper min/max labels
- check imgui's id stack logic and see if the activity change logic can't be made simpler, without flavour prop id, thanks to the stacks

- remove that heresy which is flip_flags, oh boy

- watch out for newlines in history's description of multiline textbox changes
	- appears to work out of the box

- determine what makes the atlas be regenerated when entering editor?
	- its just that zeroed cosmos has a different set of resources needed - zero
	- perhaps zeroed cosmos is guilty?

- Command multiplicity
	- Make change flavour/component commands support it out of the box, don't just have two different commands
	- Value changed will have the same format - there's always one value set
		- The old values will be a stream of old values actually
	- mark different values in orange


- Selection window
	- Proposition: have always the nodes for entity types
		- flavours there could be unified already
		- e.g.:
			[ Save selection ]
			[ ] Unified view - this actually is not much important because most of the time entity types won't be considered two at a time
			-> Shootable weapon
				Editing properties for Kek9 and Sample rifle
				-> Name
				-> Entities	
					[ ] Unified view
					-> 2 of Kek9
					-> 9 of Sample rifle
			-> Sprite decoration
				Editing properties for Fish
				-> Entities
					[ ] Unified view
		- Ctrl+Enter on selection adds to current selection
		- In case of just one entity, always unify
		- if we introduce just another node of "unified", we stay stateless

- change the scale of attenuation variables. we might later even pass doubles in the light system?

- light should have an option to rescale the distance
	- done: we can now just tweak quadratic quite easily
	- so that it just becomes bigger, without altering the attenuations
- we have a problem with too small values for light

- Mnemonic bindings for windows
	- Alt+S - selected
	- Alt+A - all entities
	- Alt+H - history
	- Alt+C - common

- add docking so we'll be able to get rid of that tab api
	- we have a bug when closing a tab with mouse


- Editor slight fixes
	- show comfortably transform of the selected
	- show comfortably number of the selected
	- show comfortably aabb of ...
	- show comfortably rect select mode


- Plan of pixel-perfect rendering
	- Let camera always render at integer positions;
		- Except when zoom is bigger than one - useful at editor, and otherwise we do not care.
		- The discarding may happen at rendering stage.
	- Let renderers not be concerned with integerization at all.
		- If something is rendered without rotation at integral positions, it will still render well.
			- If something is a dynamic body, then it might make sense for its movement to appear smoother anyway.
				- Even the player.
	- aabb drawers should always trim to pixel coordinates because they anyway draw in screen space.
		- don't make movement smooth at bigger zoom. let the grid also be rendered in accordance to where the integerized camera is
	- the character is not shaky with camera smoothing if its transform discards fracts both at rendering stage and at camera's chasing stage
	- let's just discard the fracts manually in interpolation system per each frame, for the viewed character.
		- we don't care for now what happens without interpolation. Screw it.

- Shaky/inaccurate positioning of sprites
	- Some are caused by interpolation
		- Why not just stop interpolating after some epsilon?
	- looks like tearing is solved by just integerizing camera
		- we had a case where having put view at 0.5 of pixel horizontally, we had tears in tiles 
	- old implementation appears to successfully stabilize the character
		- we need to stabilize both though
		- we'll specify an entity whose visual transform is to be snapped to the camera position
			- could be done in interpolation system?
	- problem is, if you center the camera exactly on where the player is physically, you'll get funny values
		- because physics floats aren't really accurate
	- so we have a conflict: we want camera coordinates to be integers, but at the same time want to render exactly at the player 
		- what if we only discard fract for the player characters?
			- truck though...
	- some tips? https://www.gamedev.net/forums/topic/670059-texturing-occasionally-off-by-a-pixel/
		- GL_LINEAR indeed appears to be a perfect test
	- it appears that the gaps in physical bodies are caused by calling discard_fract in drawing.cpp.
		- however, this should not be the case, because they are initially positioned at integer positions.
		- and the editor shows them correctly once their values ale slightly altered by the tweaker, clearly indicating a numerical issue.
		- try integer si?
			- if all else fails, remember we still will have to switch to a fixed-point arithmetic in box2d for networking
				- so it might be saved for later
		- the reason that the player doesnt look beautiful after centering camera on it directly and smoothing, is that the algorithm with value fields or something itself is very smooth
			- also, just discarding both at camera and drawing.cpp is not enough because th camera must properly chase the integer, so still it might be sometimes off by 1-2
				- notice that after discarding both, the "ugly transitions" are gone but there are still off by one errors, just like with the brick walls
				- might also be because of the brick wall sprite not being 128x128 exactly

- transforms in editor
	- grid
		- tips: https://developer.valvesoftware.com/wiki/Hammer_Tools_Menu
		- hotkeys for:
			- toggle ctrl does snap?
			- snap bboxes independently: ctrl+shift+b
		- valve aligns to selection's bounding box, looks like a neat idea
			- research about corners other than left-bottom
			- we'll just snap each AABB corner
				- algorithm:
					- for each aabb corner, determine which grid vertex is the closest
					- for each determined closest grid vertex, choose the one with the least distance,
						- and align the respective vertex 

- moving objects with mouse
	- if only one is selected on pressing "t", optionally move in a special context
		- like move an entry in attachment matrix
	- command format
		- how about re-using change property command?
			- no.
		- just move the requisite stuff from change property command mixin to a separate header
			- it's the art to have structures with diverse data types, each only to do their own business,
				- but have a templated logic that generalizes them all
			- entity_translator struct
				- it can hold the currently processed command
			- separate header:
				- detail_*_bytes -> write_maybe_trivial_marker
					- actually, not...
					- consider removing trivial marker completely
						- no: too much pain in the ass with various type ids and various trivial types overall
					- then we'll have a type id with just max 2 options (b2Transform, components::transform)
			- readwrite of before_change data could be used
				- because we will always store all old values for determinism
		- really watch out for the changes in "si", they can later affect how the box2d detail structs are redone
			- SI WILL BE PRESERVED CORRECTLY, because changes to it will also be tracked in history.
		- now that we have box2d transforms in a struct, we can have a lambda accessor for a single transform object
			- can set it with a constexpr
		- task at hand, data-wise:
			- write b2sweeps and b2transforms to old value vector
				- pack them into a box2d detail
			- set new transform
				- what format?
				- new transform of the reference entity?
					- then calculate delta and apply to other entities
				- the delta components::transform itself?
					- no referential entity to look up to
					- always calculate pixel-wise delta during mousemotion
						- SI-independent
			- rewriting values when moving mouse

- add a cmake flag to disable building property editor

- add has_ctrl to select_all_entities


- research why fast randomization doesnt work with single call?
	- indeed the quality is bad, we've tested

- fix 180 snapping?

- composite commands
	- some commands make sense to be redone and undone in bulk
		- e.g. paste + move
	- history implementation should remain the same.
		- it is only the case of GUI that some entries would appear as scoped nodes...
			- ...and that some entries would go with each other on just a single ctrl+z/y
- duplicate might just take ids, I guess?
- fix epsilon deviations individually after moving a selection

- move flip flags to component state and let it be drawing input?
	- generally refactor draw entity scripts


- flipping a selection
	- useful for creating symmetrical levels
	- ~~M or m would make a square from one corner~~
		- Just use ctrl+arrows. More intuitive
	- always aligns to aabbs
- add lights to tree of npo
	- just compare each against the camera, we won't have so many lights to warrant npo nodes

- wandering pixels disappeared?

- implement some stupid simple physical logic for crosshair recoil entity
	- so that a separate child entity is not needed
	- maybe even copy some from b2Body

- fix readable bytesize, test it

- Grouping & ungrouping
	- Shift + click could always highlight&select a group

- Move summary to somewhere else, even if we pass it the editor setup reference
	- what would be wrong with it?

