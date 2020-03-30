
- minimize use of b2Rot() constructor for angle, use SetIdentity

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
		- cartridge component
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
- rename bound to "bound"
- use range widgets for bounds, with proper min/max labels
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
					- then we'll have a type id with just max 2 options (b2Transform, transformr)
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
				- the delta transformr itself?
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

- Automatic group generation for duplications and mirrors

- Get rid of that hrtf folder?
	- will everybody have it though?

- add "reset" to pool that repopulates the free indirectors in descending order
	- so that refilling test scene works
	- actually "clear" does this already
- remove "implied components"
- remove build info printing from hypersomnia version
- **Chosen solution:** A folder designates a project.
	- **For now we ditch lua exports.**
	- **Project folder structure:**
	- ``ProjectName.tab`` file - the editor tab metadata. History, current camera panning, all selected entities. Designed to let you start where you left off after restarting the editor.
		- **It does not store paths.** 
	- ``ProjectName.int`` file - the meat of the map - all entity flavours (e.g. ak, pistol, sword,  confetti grenade) , floors, obstacles, decorations, also relative paths to used images, sounds etc.
	- ``autosave/`` - a directory with all unsaved files.
		- most intuitive solution
		- it's a little like git index
		- we won't have to create templates and will be able to reuse the logic easily
			- cause it's just like a different project
	- ``game_modes/`` - directory for all game modes. Editor iterates recursively through this directory and loads them into std::vector<std::pair<std::string, team_deathmatch>>.
- on saving an untitled work, we will have to move the folder.
	- And possibly rename all of (tab, int, autosave will be absent though) that is inside to match the folder's name.
	- We'd anyway need this as "save as"

- fix all logical assets to not use those crazy tuples and getters, wow

- create templates/traits folder
- always_false -> identity_templates

- make particles hold animation ids, current time and current frame number

- animations in particles
	- just hold animation id?
	- what about recalculating the frame number?
		- could be slow
		- unless we calc time elapsed and current frame in the particles
			- in this case it will be O(1)
			- just keep animation time

- assigning names to all logical assets in editor
	- simplest would probably to store a string name in each
		- it won't hinder cache much as it's just pointer + size
	- just stick to names in the assets themselves for now


- Project files vs official files
	- We should probably convert to local files only at the stage of i/o and upon displaying
		- Less pain in the ass and more performant
	- We could keep copy of the paths or just maps that were resolved?
		- This could get out of date perhaps
	- If we keep absolutes, caches will get regenerated on moving the project
		- as they keep absolute paths
		- Won't happen often, though

- We should now handle the missing images, probably
	- When should it catch up with the missing images?
	- The setup (in this case, editor) might need to trigger the reloading of viewables
		- Since it might get a clue of when the files stop being missing
	- Even if we continuously stream, we won't waste time rebuilding the atlas if all viewables stay identical
		- So triggering is still sort of needed
	- In practice, will this ever be the case that we stop streaming new atlases?
	- Not beautiful, but we could have a mutable bool last_seen_missing inside loadable
		- ~~Then it will be the job of the atlas to set it if it was missing~~
			- Actually... we should only ever do this inside the editor, because atlas might not always reload all viewables
		- Actually why not just have an unordered map with missing flags, that is cleared on whenever we destroy an asset?

- the problem of tracking images
	- By default, don't show unused images
	- on missing, we have to relink manually anyway
	- simplest case: what if the image data consisted of just paths?
		- Just like with sounds
		- Then it would not make sense to have forget buttons, right?
			- because it would only be about the path
		- so it is only the case of what do we do with neon parameters
			- it might be good to save a meta file so that on re-importing, we have that data again
				- though we'll still to have to relink due to ids
	- Images window
		- It's fast enough so always recalculate missing files on opening Images
		- First should show Missing files with red caption so that it catches attention
			- Option to relink each path in particular
		- Even earlier should go Missing custom neon maps

- error: could not redo "create_asset_id" because "some.png" could not be found.
	- we should just allow non-existing paths. **Ids should always be there.**
		- if an image does not exist, we just load a fallback blank texture!

- sound buffer input should not hold path template to be typesafe_sprintfed
	- rather, let it keep path to the first file. We'll alter check if ``_2``, ``_3`` etc. exist.
	- We'll just check if it ends with ``_1.ext``

- Images GUI: make images selectable
	- What about redirection?
		- Just don't show "Redirect path" button in the mass-selection gui
	- checkboxes would be better instead of ctrl+something
		- perhaps similarly with selection groups
		- separate column for checkboxes, simply
			- if a checkbox is on, also highlight
		- fae could use it as well but it will be a little harder to persist the state, so not for now


- importing & using images in editor
	- Simply... keep track of whatever we actually use
		- Images dialog would simply show all used images
		- We will anyway need this logic of traversing all ids
	- Invariants will look like they are picking file paths really
	- Makes no sense to always automatically import all loadables recursively from the folder
		- E.g. because we don't always want all official images imported
	- Or does it?
		- if we do, though, merely adding and removing an image may result in an error in editor
	- Importer window
		- Unimported on filesystem
			- Import next to each folder and file
		- Imported
			- shows how many flavours use an image
			- Un-importing requires to delete all using flavours
				- Which in turn requires deletion of entities with that flavour
	- What if an image for which we've set some neon map values becomes unused?
		- We don't remove it from loadables list. We simply add a "Remove" button next to it.

- how do we reload viewables that will be stored in pool?
	- do we simply compare object vectors?
		- if order changes, that accounts for a change as well because ids must point to correct things
		- do we need to compare whole pools or just objects?
			- I guess pools because identity might've changed
	- what about sparse pools?
		- it won't have indirectors, so object vector check will be enough
- also would be nice to have those vectors for animation frames as well
	- then it would be nice to at least be able to edit the introspected structs,
		- but store their whole copies


- command: use_image_path
	- won't be a standalone command really, just stored inside change flav prop command
		- like grouping command is stored
	- redo may create new entry in loadables
	- undo may destroy that entry
	- if the path exists by the time of redo, nothing is created/destroyed
		- for statelessness, let it calculate existence on each redo/undo
	- the command must store undo input and do undo allocate
- editor command: forget_image_path
	- won't check for usage, it will be the job of editor gui to not let it be called when something is in use
	- contains the forgotten viewable's content

	- makes practically no sense to have an invalid image id
		- invalid sound id could still signify a no-sound which may make sense
	- if we disallow them, we need to somehow handle this in editor
		- e.g. flavour could always choose the latest image id
		- a popup saying "before you create this flavour, import at least one image"
	- still, is there really no way that no invalid ids will take place in sprites?
		- what if we want to un-import an image from the editor?
			- do we have to delete the flavours that use it as well?
			- or do we make a popup forbidding this?
		- what if an image on disk gets deleted and an atlas is required to be regenerated?
			- then stop displaying the cosmos and just be left with imgui
			- we can check in main if the game world atlas exists
	- in stuff like gui however, we'll seriously need to check with mapped or nullptr

	- What do we do about writing image size to sprite?
		- add_shape_invariant_from_renderable can assume always found
			- cause it will be called from editor only on existent ids 
		- when switching from invalid to non-invalid, automatically write size information to both sprite and physics
			- otherwise provide a button to update sizes
			- Buttons:
				- Make 1:1
				- Write to physics
		- Images
			- Is there a point in making separation of image ids and paths visible to the author?
				- The question becomes if more than a single flavour is going to use the same image
				- Flavours will very often share logical assets though, by their definition, so it makes sense to have them identified
			- Architecturally, the flavours will store only identificators anyway.
				- So in case if shit hits the fan, we can provide the user with a textbox to change the broken paths
			- Proposition: the editor shall perform i/o upon the extras.lua and meta.lua
				- Because if we consider the game image's path to be an identificator, what happens if it gets moved?
				- It's a pain though because we will have duplicate state
			- Behaviour in sprite invariant
				- Should be displayed as simplified, instead of some mnemonic id?
					- e.g. "corner (walls/1)"
			- Automagic regeneration of shapes
			- Is more than just an image path!
				- We don't have a singular "game image" structure, though. It's divided into loadables and meta.
				- So we'll need a special purpose GUI code that connects them all
		- Sounds
			- Same problem with paths as in the editor
				- Sounds are more likely to be reused
	- Viewables
		- 
- remove quadratic number of instantiations of field comparator
	- just do augs::to_bytes on both elements to be compared
	- have some reserved thread_local memory stream

- crashfix with no images in atlas?
	Regenerating texture atlas: cache/atlases/game_world_atlas.bin
	Loading font content/necessary/fonts/unifont.ttf
	ensure_less(static_cast<size_type>(i), count) failed with expansion:
	9 < 0
	file: ../../src/augs/misc/constant_size_vector.h
	line: 103

	- is cosmos::zero involved?
	- it's actually because we are changing the cosmos while some particles are still there to be simulated
		- and the default intercosm has no ids for the particles
		- so we must reset the particles when we switch the cosmos
- replace decay_t usage with remove_cref, decay is overkill

- flavours and entities are concepts disparate enough to warrant separate trees completely
	- same code can handle them, though
	- or is one subset of another?
	- all are the same, except that
		- checkbox target differs
			- actually for entities we may also make them selected?
		- entities browser does not view the flavour properties
		- flavour browser does not view its entities
	- we won't have separate windows, just a radio box at the bottom

	
- Rethinking FAE tree gui
	- By default, the "all" view for Entities should always have selected entities ticked.
	- Should fae tree abstract away the tick behaviour?
	- In case someone wants to edit only several entities from the selection, 
	alt+s will have tickable boxes that are not related to the current selection state.
		- Implementation
			- Four cases of usage
			- All entities and ticks = selection
			- All flavours plus some manipulation options, ticks = edit selection
			- Selected entities and ticks = edit selection
			- Selected flavours plus some manipulation options, ticks = edit selection
			- All can be filtered
			- Therefore little sense to make a special case for all entities?
			- Common elements
				- All are structured like Type->Flavours->(different things)
				- But the checkboxes differ in meaning
	- When to
		- Disable type
			- All (flavours)
				- Zero flavours => Zero entities
			- All (entities)
				- Zero entities
			- Selected (flavours)
				- Never
			- Selected (entities)
				- Never
		- Skip type
			- All (flavours)
				- Filtered out
			- All (entities)
				- Filtered out
			- Selected (flavours)
				- Filtered out
				- Zero entities <=> Zero flavours
			- Selected (entities)
				- Filtered out
				- Zero entities <=> Zero flavours
		- Skip flavour
		- Disable flavour
- fix broken file combos
	- they break when two are visible at the same time
	- so something with ids

- fix gui in images
	- F next to name instead of forget far away
		- tooltip showing how many will be forgotten

- for_each_id_and_object -> for_each_id_and_object, and move it to some range templates
- Fix glitch with flicker on the entire screen
	- probably particles animation bug

- use pools and pool ids for:
	- all logical assets
	- viewables
	- flavours
		- later will be sparse pools where performance requires it

- REALLY enable these clang warnings
- fix crash when changin neon map color

- really make entity_flavours just a direct pool
- for_each_container usage for ent pools

- start moving instantiated entity right away
- finish stuff with flavours, disallow removing used ones

- create gfx and sfx folders for the project on saving
- Wextra for gcc?
- how do we communicate the used locations of flavours?

- can flavours ever have invalid ids?
	- Yes, then they may signify that no entity is to be spawned.
	- However, if a flavour id is non-zero, it is guaranteed to point to a valid flavour.
		- That is because once again we will not allow the editor to remove flavours of whom there are users!

- add some warnings to clang
- perhaps optimize pack.cpp somehow?
	- can't these nodes be linearly allocated?

- enable neon on charges shells etc
- don't dwell at rectpack2D for now.
	- For what it's supposed to do, it is good enough for now..
		- We'll make it pretty when we're done with Hypersomnia.
	- firstly let's introduce the true usecase
	- the unstaged changes can sit there really
		- we don't often delete this folder and it's not critical

- morning refactor: separate atlas profiler

- there is no point in having a convex partitioned shape inside an image cache
	- that is because these shapes will be edited in editor, possibly per-flavour

- sort images for multiprocessing; biggest go first

- refactor augs::image_view to use common funcs?

- test out gdbgui with clang build as well

- tests of traits: check no flavour ids in entities
	- we could not possibly afford looking through all for usage checks

- tests of traits: check no ids in invariants, at least
	- screw initial components really

- Footstep sounds & particle effects
	- Movement system just posts faf effects
	- Effect happens when:
		- animation amount has just entered an edge frame
		- the velocity is greater than some threshold 

- Looks like old offsets are still dangling after performing transfer
	- if for example bare is unset, but akimbo is set
	- fixed by reinferring whole capability trees

- Don't inherit in entity_id, just store the base as "raw"
	- Exactly as in flavours - this works well.

- Fix magazine offsets


- Rethinking attachment offsets
	- Don't use a matrix. It will introduce shitload of redundancy
	- Solution:
		- In item invariant, store the holding offset (from center)
			- Maybe also the back holding offset, for backpacks
		- Each torso animation specifies hand positions
			- Maybe also the back positions, for carrying backpacks
		- Do the math
		- Make it very specific, not per slot type
	- Contextual positioning - still useful?
		- Probably not if we have good previews in animation
	- We will need reinference when modifying animation state,
		- as we will probably read the offsets from the relevant torso invariants
			- for now, we'll always just take the first frame and be done with it.
			- we'll also reinfer upon switching the armour
	- Drawing considerations
		- What about viewing transforms?
			- If we draw with sentiences, not subject to interpolation - and correctly
			- Singular function to calculate the current animation frame
				- Will anyways be used in more than just one place
					- E.g. on calculating the collider connection, we'll take the first frame
		- If we draw held item sprites separately, we must calculate the torso frame twice
			- We'd go bottom-up everytime... for each attachment, etc.
			- That sort of sucks
			- Instead we might draw the entire character if we determine that it has torso
			- And avoid drawing anything if the owner capability has a torso


- Specyfing hotspots on the images
	- State considerations
		- Image definition shall store image_logical_meta 
			- Each metric type will be specified with maybe
		- On modification in Images GUI, they will be rewritten into cosmos common state
			- And cosmos will be thus reinferred
		- Our state will be duplicated, but it is not really bad
	- Cases
		- Bullet spawn offsets
		- Head offsets
		- Primary/secondary hand offsets
		- Back offsets (for backpacks)

- Schedule steam outburst effect if max heat exceeds ~80%

- Gun engine sounds and car engine sounds
	- This case is analogous to wandering pixels, for example
	- This would be useful to consider in networking context already
	- Messaging in step vs polling
		- Polling might not be that bad performance-wise if we only ever consider entities near camera
		- And it is the most robust approach that will respond with correct results to arbitrarily passed cosmoi
	- Implementation should just use message responding logic to create specialized caches
	- just make regular audiovisual caches...

- Complex ImGui controls in general property editor
	- In fae tree:
		- Flavours
		- Entities
		- Recoil players
			- Name-able, linear combo box
		- Physical materials
			- Name-able, linear combo box

- for now do an undo_delete_entity test with a floor and some walls perhaps
- storage format for deleted entities

- For continuous sounds, sound system should probably assume the same strategy as an inferred cache.

- animation in our architecture
	- should work statelessly, in particular it should not set values to sprite.
	- callbacks
		- normally, one would have std::function per each frame.
		- what we will to is to store a variant per each animation frame.
			- even if the variant is big, the animation will be an asset anyway.
		- then the animation system will perform the logic and e.g. spawn particles and sounds.
		- we won't separate concerns here because we're not writing an engine, rather a game.

- properly templatize entity_id_base, add entity type's key to it
- Implement a color picker inside the neon map light color chooser
	- less pain in the ass
	- look for imgui logic to acquire mouse positioning relative to the control

- Support for animation import
	- Not important until deathmatch as we will probably setup testbed animations for both parties, programatically
	- Combo with text "Import..."
	- Button inside each animation entry - Reread frames and durations
	- Deleting and names
		- Also delete automatically when last frame is removed?
			- Would make sense if the name itself was not a datum

- Animation architecture
	- Animation asset
		- Several distinct types of animations
			- Each animation will be treated as a completely different asset
				- Then we will never std::visit contextually, because we will always contextually know what kind of animation we are after
				- Mass selection just like anything else
				- Just the editor's window for animations will just hold all types separated
			- Might be a little pain in the ass to setup
			- Actual types
				- Plain animation
					- Just list of frames w/durations
					- Suitable for decorations or for particles
				- Animation for the gun being shot
					- Can actually be a plain animation, can't it?
					- The sprite chosen for the gun will be the gun in its idle state
					- Later maybe an animation for being idle?
						- Gun would hold both ids
						- Rendering routine could just see if these ids are set
				- General: Torso animation
					- Will only have base speed ms, as game will determine pacing for each frame
					- Frame state: 
						- torso metrics
							- vec2i hand position[2]
							- positions of these could be even indicated in the previewed image
				- Shooting torso animation
					- Can have duration per frame, no problem
					- Frame state: 
						- torso metrics
							- vec2i hand position[2]
							- positions of these could be even indicated in the previewed image
				- Moving legs animation
					- Will only have base speed ms, as game will determine pacing for each frame
					- Frame state
						- We won't implement these just yet. Won't matter until after deathmatch stage
						- leg metrics
							- maybe<vec2i> foot_position[2]
						- is step, or just assume that the last frame denotes a step
					- Then the sentience invariant specifies particle effect to play on step
						- Or the common state holds a map of ground material types to step sounds
							- This could be an asset as well
				- Implementation
					- We shall begin with movement animation and then repeat the code for the rest of types?
		- Identification in editor
			- Architecturally, animation is an unpathed asset
			- Visibly, it is always tied to some path designating the first animation frame
				- Screw that. Animation will have enough state to warrant just picking what is created already.
			- Do we open a path dialog when choosing an animation?
				- No.
			- Displayed name is immutable either way - always the name of the first frame.
				- We don't care for now that displayed names can be duplicated.
		- An animation frame of any type will guarantee only the image id to exist
		- Animation asset state may be in this case important for game state
			- E.g. hand metrics could determine swing trajectories
			- E.g. foot metrics could determine sound events
			- Thus we make animations logical assets
				- Nothing wrong with it, is there?
					- It will anyway stay immutable. The only practical difference is in which code can access what.
					- Later, we might introduce mutable states for these animations as well, and they will act like flavours
						- Though it will be discouraged for the sake of statelessness
	- In solvable
		- Each torso specifies a set of animations that it uses
			- Torso invariant
				- forward legs animation id
				- strafing legs animation id (later)
				- walk animation id
				- punch animation id (later)
				- rifle carry animation id
					- No problem with hand offset being rotated a little.
				- rifle shoot animation id
				- pistol carry animation id
				- pistol shoot animation id
				- akimbo carry animation id
					- Will be same both for pistols and rifles
				- akimbo shoot - only a half
					- after shot animation is complete, we should return to akimbo carry animation with 0 offset
			- Character flavour will have its own torso invariant, which would denote the basic armour - when nothing else is worn
				- The animations shall depend on the torso invariant of the item in the armour slot
			- draw_entity finds the relevant armour invariant
		- Movement component shall expose "movement amount" from whose the torso frame number will be calculated statelessly
	- Interaction with item attachment offsets
		- item invariant should specify hand offset, which will determine relative positioning to the hand

- on_dynamic_content rename 

- Simpler solution: Sequence view
	- PGUP/PGDOWN just move to the next id searching through images defs
		- with help of cut number and get number at end
			- actually *maybe* no: might be useful to have pgup/pgdown for filtered outputs as it will also
			avoid the need to click the found item with mouse; just always pgup/pgdown for the next found
	- V button next to tickbox to enable viewing if property column is enabled
		- make property column disabled by default then
	- VIEWING textual preffix
	- node'ize (instead of combo'ize) the offset/color pickers so that multiple offsets can be viewed at once


- disallow removing last frame from the animation and always require to create an animation from an existing set of frames
	- simplifies calculations
- Fix imperfection when duplicating vertically
	- will b problematic for both axes

- Add slight, disappearing (by shrinking) particles for glass impact
	- What if we could sometime make it so that they stay? And we could synchronize permanently sitting particles through the network?

- Bullet destruction remnants
	- Simplest solution: spawn some particles
		- Pro: lots of already implemented control over spawning offsets and rotations
		- Pro: less of things to do in physics

- Simple rect node classes for automatic positioning of scene elements from code
	- Create metadata along with each size passed
		- Later it can be instantiated
	- Simple struct, cascade nodes, some state?
	- Can later create entities from nodes returned at some point

- Set direct listener for mono sources like this:
	alSourcei (sourceName, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f (sourceName, AL_POSITION, 0.0f, 0.0f, 0.0f);
	alSource3f (sourceName, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	- Though I guess it's easier to just set the position to listener position?
		- How about doppler?

- Add humming sound effect for the aquarium
- Fading implementation is not necessary for now, just overlaying

- Continuous sound design
	- State
		- invariants::continuous_sound
		- Pro having an immaterial, sprite-less sound type:
			- There will be a lot more plain sprite decorations than sounds, so it makes no sense to attach a sound to each sprite
			- Even if it is connected with a decoration, the origin of the sound might not always necessarily equal the sprite origin
			- It might be nice to have a separate sound icon to distinguish sound entities
			- Anyways we can always group them in the editor
		- Pro having a sound next to sprite decoration:
			- Origin is the same? Which is sort of a con, actually.
		- Add it to the complex decoration as well? Or should it just have a humming sound like sentience?
	- Solution: a simple audiovisual cache that just plays the sound when it sees it for the first time
		- How does logic differ here from firearm engine caches or sentience humming caches?
	- Solution: an audiovisual cache synchronized against when_born of an entity
		- Proper for music synchronization
			- Would only periodically check if the music hasn't diverged from the timing
		- Proper for continuous spatial sounds whose doppler is disabled
			- Could be considered music as well?
		- Problem: some continuous sounds might be subject to doppler, so when_born of an entity no longer applies
			- Notice that network synchronization is more or less impossible with doppler sources, because each client listener has its own frame of reference
				- Which is not a problem because these sounds will mostly be short FaFs
			- Simply disable doppler for this kind of sounds and the first solution applies in this case
				- set_doppler_factor for a source
- Priority: Finishing aquarium showcase
	- Bubbles appearing from fish
		- Complex decorations that get deleted after animation passes
			- Pro: high amount of control when they are spawned
			- Con: more entities? Who cares, though.
			- Solution: delete_entity_after_loops in invariants::animation
	- Directional bubble stream at top of the aquarium indicating the water source

	
- Statefulness in case of sprites
	- Processing them all in the game logic:
		- Pro: easy peasy
		- Con: server has loads of unnecessary work to do
		- Con: visuals dependent on tickrate
		- Overall, sucks
	- State immutable at drawing stage, advancing only visible along with other audiovisuals
		- Pro: works and architecturally pure
'		- Con: Performance hit: we must dispatch logic twice to all visible entities
			- Don't we do that anyway?
			- This happens with wandering pixels but they have dedicated layers
				- Sprites can be pretty much on all layers and only several will have the special effects enabled
		- Con: Must somehow get rid of invisible ones or caches will grow?
			- Must anyway be prepared for the worst case
			- Currently the case for wandering pixels as well anyway
	- State mutable at drawing stage, advancing while drawing
		- Pro: best performance, don't have to iterate twice through the entire 
		- Pro: even easier than with state-immutable approach, less funcs
		- Con: architecturally impure, has to pass delta for illuminated rendering
		- Con: Still has to somehow clear unused caches? It will only affect memory, though, and will never be more than max number of entities of all cosmoi
			- Same problem as if the state was immutable


- Fix: item rendering in GUI, as it uses some sprite internals so it could break
	- Use draw_entity somehow for drawing items along with attachments

- Remove the need to specify all frames in test scene images
	- As well as in test animations


- it would be nice to store the sprite size information per entity
	- Solution: Store overridden size as a value with flag in a separate component.
		- Pro: On changed texture, size in invariant is updated as always, whereas overridden sizes are left untouched.
			- Pro: literally no change to the update logic!
		- Pro: fastest condition resolution as it it will require only a boolean check.
		- Will be set in editor whenever dragging happens.
	- Solution: Infer physical shape from overridden size, invariants::sprite::size and polygonal shape specified in the image's logical asset.
		- Just partition image's polygonal shapes into convexes when writing to logical assets, to speed up reinference.
		- Only infer from shape polygon if it exists as a component.
			- Then it will probably also be a vector.
		- Don't store shape invariant in things like character, item, plain sprited body.
	- In crosshair component, size for appearance is anyway not needed and the original one will be passed in the drawing code.

- sprite will have to know which tiles exactly are visible in the camera
- Light optimization: remove vertices that are determined to be behind some convex shape.
- Remove the need to keep shape polygon for most entities
	- Less work for size updater, we'll just reinfer

- ltrb_grid_intersection
- move rng to the audiovisual state to avoid having so many tls variables

- rename the flavour on setting an image
	- if it is detected that it is yet unnamed

- Icons: let them be rotated.
	- drawer should have a routine for drawing rotated aabb border?

- Editor: Markers
	- Whichever has a spatial representation shall be represented by an entity.
		- Pro: Re-uses the logic for resizing, moving and rotating.
			- Perhaps even for shapes later on.
		- Pro: Re-uses logic for icons.
	- New entity types:
		- Point marker
			- Has a transform so it can be rotated as well
				- Useful for spawns
			- icon should rotate along the rotation
				- aabb will be enlarged then
		- Area marker types
			- Shape
			- Circle
		- marker_meta
			- associated_faction
		- invariants::point_marker
			- point_marker_purpose enum
				- spawnpoint
				- resistance_spawn
				- atlantis_spawn
		- invariants::area_marker
			- area_marker_purpose
				- bombsite_a
				- bombsite_b
		- Additional metadata for a marker
			- We'll for now stick with a struct that holds all possible properties
			- There are several problems with using a variant:
				- We would have to implement it ourselves to guarantee:
					- Portability
					- Preservation of destroyed values on later revival
						- Like with augs::maybe
					- Well, technically, we could revive the trivially_copyable_variant from deleted files
				- Quite a lot of bloat for types
				- Thus we'd lose a lot of valuable time

- get_max_charges_in
	- num_fitting_in

- move access_each_property definition to change_property_command with if constexprs to remove cosmic function dependency

- optimize cosmos headers
	- separate handle getters

- Propagating sounds from the mode logic
	- MidStep callback?
		- How do we define where it is called?
		- Just before deletions?
		- What we might want to post?
			- Is it important?
		- For greatest flexibility, shouldn't we, in the mode, process the cosmos in presolve?
			- That means we input messages for the cosmos.
		- And check win conditions in post solve.

	
- Draw use button range indicator with dashed lines
	- wheel section
		- check for collision just like with the explosions

- Implement bomb-bombsite overlap test
- Add more sounds to the bomb

- Interrupt the started arming sounds on every play request
- Fix switching between the weapon and the bomb


- Fix defusing logic
	- Use button logic will need some improvement 
	- Use button should be queried continuously
		- Preoccupation tracking
			- Once the use button has discovered something to use, it should be locked, i.e. it shouldn't take hold of a car wheel while it is defusing
			- enum for the use button state?
				- IDLE, QUERYING and current preoccupations, e.g.: DEFUSING
				- Since there can be always just a single state at a time, a single enum is required
				- When preoccupation is interrupted (e.g. due to distance), it is always set back to QUERYING
				- When use button is released, it is set to IDLE
				- In case of a single-shot action, e.g. taking hold of wheel, it is always set back to IDLE so that another press is required
		- Different domains might interpret it on different occasions
		- Centralize this completely?
			- Treat is similarly to trigger
			- Anyways we don't need the wheel handling for now
				- Why would we respond to pre-solves there anyway?
		- Set use button to true inside sentience and perform some general logic upon it in sentience system, all in one place
			- Prioritize bombs, obviously 

- New entity type: Explosion body
	- invariants::explosive shall contain a cascade explosion input parameters
		- this will be adequately set for C4, leaving grenades intact
		- several explosive_body flavours?
			- each flavour might specify several explosion types from which to randomize
			- plus a mult range for randomization of size
	- Has a circle shape with high restitution
		- Can even draw some highlights in place of the circle, for a cool effect
	- Has a pathfinding_query filter that only collides with walls and other concrete objects
	- Periodically spawns an explosion
		- Applies a slight, random force each time
	- removes the need to introduce global solvable, also will be more easily networkable in case of an MMO

- Implementing the bomb
	- Global solvable
		- Priority queue with scheduled explosions
		- Universal solve that is used by all solvers, standard or others
	- Bomb should have a backpack category and be too heavy to be put into backpack

- Bomb logic
	- Arming counter is increased when:
		- Player is inside the bombsite area
		- Player has all movement flags unset (except shift for sprint)
	- Arming counter is reset when any of WSAD flags goes true
	- When placed, change C4 to a static body with filter that only collides with bullet shells
	- Unarming counter is increased when:
		- Player is inside the bombsite area
		- Player has all movement flags unset (except shift for sprint)
	- arming/unarming durations inside explosive invariant
	- defusable_by boolset with factions


- calc participating faction could calculate bombing and defusing right away
	- we'll also avoid having three of them
	- it could constexpr after the type of input (thus the type of the mode)

- don't transfer bombs across rounds

- 3 magazine slots on torso so we don't have to buy backpack for some already viable setups

- bomb_defusal is inherently two-faction, rules don't make sense for three
	- but both factions can have bombs, why not! Let's code a generalization straight away.
		- actually, let's not do this for now
		- calculate the bombing faction by finding the first bombsite marker
			- spawn a bomb per each round for a randomly selected player of the bombing faction
			- bombing faction wins
				- its bomb stops existing (it exploded)
			- bombing faction loses if
				- its bomb gets defused
		- after time limit
			- bombing faction loses


- damage_origin
	- damage_sender
		- direct sender
			- e.g. a gun
			- in case of a punch, the character
	- damage_origin = damage_sender + []
		- inflictor entity
			- may be a bullet
		- entity-less inflictor
			- spell
			- punch trigger

- let a random character pick the bomb
- Make a command for adding a player in editor? 

- add large_number_font with just numbers

- Fixing the arena mode GUI
	- We'll moving from ImGui to our own GUI to have finer control.
		- We can offload the work to ImGui when the player is actually selected.
		- The buttons for the players will be highly deterministic since it is really an easy layout.

- separate field address types in property editor
- Fix smoke traces for steel bullets

- Mode helpers
	- for_each_character?
		- actually we'll just for now iterate over all sentiences
	- find_player_flavour(faction_type) for player creation

- Test scene game mode
	- The first to be implemented as an architectural draft
	- Test functionality: respawns every character every time it dies or loses consciousness (e.g. after 3 secs), just to a different spawn point

- There will ALWAYS be some game mode whenever a game is running
	- E.g. test scene mode

- Knockouts should be counted instantly, just cancel them when the player is revived
	- For killing a knocked out player, there is only an assist without killer - which is only +1 point
		- E.g. (+ Pythagoras) m4a1 Shuncio
	- So at most, one can get a single KO and an assist for a player
		- Actually don't count it as an assist if you knocked it out earlier
			- So how do we display it?
		- In total, a killed player can give away a single ko and two assists

- After round restart,
	- Propagate movement flags!
	- Propagate all owned items
		- Keeping GUI buttons intact
			- messages::changed_identities
				- A generalization. Might later be used for other things.
					- In this case, a map of items to new items
		- Actually transfering the items to the new cosmos
			- **Chosen Solution #2**: Serialize inventory tree and rewrite.
				- Pro: Most performant.
				- Pro: separation of concerns?
				- Maybe even less error prone.
				- Later might be handy to iterate through the already generated tree.
	- Different strategy for restarting?
		- Just reset transforms of all players
		- Set them to full

- Send a notification to the game gui about the change in entity ids of hotbar-assigned items

- Modes aren't concerned with the currently viewed entity
	- except test scene mode

- Testbed should populate the test scene mode profile that it is passed

- basic_team_mode
	- round time
	- number of rounds

- Gameplay mode logic shall detect if a map has enough information to be played in a specific mode

- special-purpose logic for markers vs entities
	- it might anyway later come in handy to visualize rects and points
	- so we might just as well reuse that logic for markers
- a hotkey should toggle visibility of **game mode markers**
- Editor maximum ease of access
	- Fix problems with grouping on duplication?

- Interoperation of cosmos logic and data defined in game mode properties

- Starting a mode
	- test mode
		- does nothing, really
	- bomb mode
		- needs complete input with the initial solvable as well, so let's first implement this

- explosions -> standard_explosion

- let players spawn for a moment after beginning the round
	- though not after changing a team

- Halftime and end of match

- Holstering logic needs to avoid mounted slots


- Mounting
	- Iteration
		- **Solution:** unordered map from an item to a target slot & params for mounting
			- Pro: best memory performance
			- Pro: best time complexity
			- Con: iterating map might be ineffective compared to a vector
				- if we use vector, then rendering code is handicapped
				- we'll go with a map for now
	- If an item is in a mounted slot, it is inferred that it is mounted.
		- And it is inferred that the mounting_progress denotes the unmounting progress
		- Since an item can only ever be mounted xor unmounted at the same time
	- Can change target slot to dead without interrupting the progress
	- As always, implement representation early
		- Draw small hud circle in the center of a mounted and unmounted item
		- Special icons? 
			- A magazine icon next to the reloaded mag
				- Not now, though

- Reloading
	- Essential to gameplay, really, so best to do it early
	- Do we implement entire mounting system? Maybe it won't be so hard after all
	- Also, chambering the gun
		- Should have a separate sound in gun invariant
		- Should always happen automatically when one hand is free and the other holds the gun
			- Is an exception to the rule of mounting that both items need to be in hand
			- For now, can play the shooting animation for chambering


- Some additional slots for magazines in case we don't have a backpack
	- Do we make it an item deposit of the player or a personal deposit item?
		- PERSONAL_ITEM_DEPOSIT pros:
			- **(HUGE)** Works out of box in GUI
			- Can later make it an actual item, purchasable
		- PERSONAL_ITEM_DEPOSIT cons: 
			- Have to create an additional, actual item entity for each player
				- Item without physical body?
				- Cons
					- More memory wasted
						- Won't take more than a single shell, though
			- Corner cases
				- (negligible) we don't want to assign it to hotbar nor want it to participate in selection setups
					- actually that's easy because selection groups only ever look in hotbar items
				- (negligible) we want to drop all items from personal deposit, not the personal deposit item
		- ITEM DEPOSIT pros:
			- Works out of box with hotbar
			- (negligible) Works out of box with drop_from_all_slots
			- Dont have to create any new item
		- ITEM DEPOSIT cons:
			- **(HUGE)** Some shit corner cases in GUI
	- Option: Several additional actual magazine slots in the torso
	- Option: A big pocket slot for the torso and several slots inside
	- Either way, we introduce several additional item deposit slots
	- Why not have a single item deposit?
		- A basic player deposit
		- Some small amount of space available

- Solution: A bool reloading_intent in the capability
	- Less traffic to the server
	- Better resistance to lag, I guess

- Every frame, iterate whole inventory and check if the hotbar has it all?
	- So that we don't have to post messages
	- Not really. The player won't always want to have the entire hotbar loaded with items.
		- Especially if they want to quickly use recent items.
	- Actually, every frame check if hands have an unhotbared item
	- And every time that find holstering slot is used, post a message

- Reloading sounds
	- Probably some delays for mounts?
		- What about making the sound itself delayed?
		- We'll be interrupting it anyway

- Reloading
	- Initialization
		- An intent arrives
			- If context exists, check if the mag can be dropped
	- Advancing
		- Check if the transfer is complete?
	- After reloading
		- The only case when we restore some state after reloading is if we have akimbo
			- The other item does not even have to be a weapon particularly
				- it's enough that it's supposed to be hidden
	- Storing reloading context
		- When to break it?
			- When the new calculated differs anyhow
		- Followup weapon for akimbo
	- When to break the reloading intent? E.g. in CS, reloading stops when a weapon is switched
		- Store entity_id last_reload_target
		- Every step when reload intent is set, calculate the available reloading context
			- if last_reload_target is unset/dead, overwrite
i			- if the newly calculated target is different than last_reload_target, reset reloading_intent and unset last_reload_target
	- Finding the fullest magaizne should be easy enough task
	- Just what if someone drops it during reload?
		- Then that's their problem
		- There's no way that an inventory structure can be altered without the user's intervention, except for player's death
		- So it can be perfectly the job of GUI to select suitable transfers.
	- The first time that the mounting conditions fail for the reloading, we're resetting it to false

- Remove the loaded mags from the hotbar?
	- So when holstering, we'll need to check if the item is in hotbar already and if not, assign a new slot
	- Actually just don't show in hotbar the items that are not currently wieldable


- Separate sounds for starting of mounting and ending of mounting
	- Same for unmounting
- Also add some sounds for wielding magazines for even better quality audio feedback

- Determinism: Fix pending mounts to have deterministic iteration order
	- E.g. make it a map and iterate by entity GUID

- FOV
	- Stencil buffer
	- We can even write a fov shader that makes a circle


- Reloading animation sequence
	- Logical order
		- Start unmounting mag
		- After delay: 3 transfers at the same time
			- Hide old
			- Pull out new
			- Start mounting new
		- Problem: The mag, once unmounted, gets hidden instantaneously
			- On the other hand, it would be nice to synchronize the end of unmounting with audio feedback for successul removal of the magazine
	- Effect order:
		- Start unmounting mag. Initiate unload animation.
		- Until mag successfully unmounted, play last two animation frames (done on the animation level).
		- Initiate load animation


- GUI required by modes
	- Arena GUI
		- Used by bomb modes, TDM modes etc.
			- May use some if constexprs internally
		- It's not synchronized, nor is it performance critical...
			- ...so let's screw that strong typing and have just one arena_gui type
		- Elements
			- Round time
				- Read only
			- Recent kills
				- Read only
			- TAB menu
				- State: bool show
				- Interactive
			- Team selection
				- State: bool show
				- Interactive
			- General
				- Read-onlies are drawn with our own GUI, from within draw_custom_gui of each setup concerned

- Fix collision sounds
	- Allow up to n collision sounds in a quick succession, for a pair of entities
		- Parameter: n
		- Parameter: cooldown interval
			- Somewhere around 100ms
		- Always reset the cooldown timer if another one happens
		- Do we need to track transforms for this?
			- Possibly not, because e.g. a shell might enter a pathological cycle and spawn loads of sounds
			- Also entities are assumed to be convex
	- Set or map?
		- unordered set
	- Kept where?
		- In the sound system
		- The sound start should contain information about if it was from a collision

- Drop items from hotbar on pressing RPM

- Buy menu
	- Layout considerations
		- How to show how many left can be bought?
		- In case of a mag, simple enough.
			- Look for hands + containers
		- In case of a weapon
			- Look for hands + containers
		- What if (+3 mags)
			- Count just the weapon
			- But we can't predict the result with 3 additional mags
		- Basically always the same slot opts

- Specifying physical shapes per-image
	- std::vector<vec2i> in the image

- Each game mode definition file will be named after its stem, and the extension will determine its type
	- **(Implemented first)** struct team_deathmatch
		- (Implemented first) player flavour id
		- (Implemented first) vector of spawn transforms
		- std optional with preferred duel transforms
	- struct free_for_all
		- player flavour id
		- vector of spawn transforms
	- struct bomb_defuse
		- player flavour id
		- c4 flavour id
		- vector of spawn transforms
		- vector of xywh rects signifying bombsites
	- struct capture_the_flag
		- player flavour id
		- vector of spawn transforms
		- flag flavour id
		- flag base flavour id
		- flag positions
	- struct mmo_server
		- start player flavour 

- Image preview in Images GUI
	- Store the image and texture inside editor structure so that it may be properly cleaned up
	- Then send just image id to the imgui renderering routine

- add this maybe? https://github.com/jpakkane/naturalsort

- Add grenades to the buy menu
- Marker entity: movement path origin

- Transform design
	- It is the case that many entities might share identical origin, in which case it would be unwieldy to update origins for all entities to a new one.
		- E.g. fish in aquarium.
	- It is also the case that the origins might be tied to decorational entities.
		- E.g. aquarium sand.
	- movement path component will have an origin transform which will automatically be moved by the editor
		- the transform component will be kept up to date and it will be the logical transform
		- the rendering code will also only touch this logical transform
	- for editor, the origin could just also be accessed as an independent transform
		- so access_independent_transform -> access_independent_transforms
		- con: more memory wasted? who gives a heck, though...

- Somehow communicate the number of magazines available and their fillings
- Maybe just automatically arrange the items in inventory gui and keep containers open by default?

- Grenades shall change shape to circle so that throws can be precise
	- Let hand fuse invariant have an optional radius for the thrown grenade
		- should we reinfer when tweaking?
	- the physics world cache will simply take into consideration if the hand fuse was released, when calculating shape
		- so hand fuse component might want to become a synchronizable

- traces
	- maybe traces should be audiovisual?
	- fix the feel of traces (maybe shrink them only horizontally?)
		- **only do this after we have editor**, obviously
	- should be fast enough
	- otherwise make it a super quick cache?
	- chosen_lengthening_duration_ms should be randomized statelessly with help of guid
	- store just stepped timestamp of when the trace was fired instead of incrementing the passed time 

- Halftime and round limit

- Add a flag to build editor or not
	- Because it builds really long

- rename world, cosmos to cosm everywhere

- Parties and hostile parties are currently integers; use bitsets properly
	- Won't matter until after we have AI
	- although, handle slot categories properly as well

- Editing vertices in editor
	- A generic "unmap_entity_vertices" and "map_entity_vertices" that depend on the context
		- Will be important later, once we want some crazy irregular maps.
		- e.g. setting reach for wandering pixels
		- if fixture, change shape

- handle mouse glitches when letting go of moved entities or duplicated ones
	- reset some drag/press state etc.


- Mode round ecology
	- Example procedure:
		- Server starts with no players, the mode gets initialized and just advances.
			- Special win condition logic for when there are no players.
		- Somebody connects mid-round.
			- They have to choose a faction, if so the server allows.
				- Should this also be mode logic?
				- We might later introduce auto-balance, in which case it might be good for it to stay deterministic.
			- Changing of faction shall be part of mode entropy.
			- Mode could keep track of names to be spawned.
			- Simply add_player({ ... }, nickname) on connection.
				- This first makes them a spectator.

- fix wielding sounds
	- keep vector of perform transfer results with marks whether they should happen
	- always notify logic, only finally notify audiovisuals

- Fix convex partitioning bug
- Fix triangle bug in visibility system

- When a held entity is visible, add its capability to the visible entities so it is also drawn

- Let us comfortably drop the item in secondary hand
	- Alt+G?
	- The most recent?

- a list of predefined filters
	- might choose a name, just like enums
	- stored in common assets

- Initial components
	- Let's ditch templatization for now
	- Entity ids in components
		- Will always be unset on creation
		- We'll just ignore them in editor
	- Useful for some wandering pixels flavours

- ctrl+scroll could disable/enable layers starting from the topmost one

- Fix what happens when too many entities are created
	- **Let the game work when a new entity cannot be created.**
		- Just return a dead handle.

- Editor status bar
	- Won't matter until after deathmatch stage
	- But will vastly improve experience
	- Downmost left for status
	- Downmost right for zoom
	- Messages in right down corner
	- Check how it works in vim
		- Changing a mode to normal clears the last message
	- Will be useful for the author to know what is going on


	- For resetting a round, the modes will need to have an initial comos solvable
		- For round-less modes, e.g. test scene mode, we can simply pass the same cosmos
			- Actually just have a constexpr bool for each
		- To re-create characters, we'll need to keep track of all of them
			- Better than to transfer from the existing cosmos as it will make it deterministic
			- Plus we'll be able to delete dead player entities if for some reason we need it

	- Keeping track of character-client relation after round is reset?
		- Solution: mode_player_id
		- Server keeps client-entity_guid pairs?
			- How do we make that entity_guid invariant?
			- Perhaps we should choose a unique in-game name for each and use it as an id?
				- components::text_details
					- instance name
					- actually a map would be better as it will be a frequent use case and we shouldn't add this to all entities
					- clients will simply query the map to find the viewing character

	- A mode shall operate without crash on virtually any cosmos
		- Should catch up with changes

			- problem: if, on delete, we remove an entity from the group it belongs to, the undoing of delete doesn't know what to do
				- grouping will be tracked by history
				- thus let the delete command just store its own "ungroup entities" command and invoke it beforehand on all entries

- Always perform autosave when starting the playtest?
	- Okay, why not just force autosave when the ensure fails instead of always doing it?
		- Automagically handles the write of last step
		- Just let the player know if it happens during step so that it...
		- okay, do we save the step to the map before or after it is performed?
		- save before, but increment the counter *after* it is done
		- in this way we'll have the player just shifted back always one step before the crash
	- Then also equip the player with ability to always write the steps to file
		- In this way when we have a crash, we can always repro easily
	- A failed ensure should write-out the step buffer so we don't have to write every time
		- We'll test this with some simulated crash later on

	- To have total determinism, just always reinfer at snapshot times, even when recording
		- This way we don't have to store inferred state in snapshots
		- This will also be a good way to test stability of the simulation under a spontaneous need to reinfer
			- e.g. on a new connection


- Snapshots during recording
	- and serialize the solvable to save on space storage
		- it will also probably be faster to serialize to bytes than to recreate entire containers with some complex maps, should there be any
			- e.g. in pending mounts
		- deserialization will be slower but it won't be used as often

	- Cases of invalidation:
		- **Undoing a command that introduces new entities**
			- E.g. one that creates an entity from nothing
			- Look for both undo_last_create and "delete_entity"
		- We sometimes completely purge, but it's only for:
			- Fills;
			- Commands whose undoing or redoing should automatically select affected entities.
				- Instantiation, duplication etc.
				- Delete has no reason to purge selections of some other entities.
		- Redoing a delete command
		- Advancing the cosmos solvable

- Disable history gui seeking to revisions past the start


- Persistence of entity ids in editor and clearing them
	- Where entity ids are in the editor state:
		- Selection
			- Rectangular
			- Significant
		- Selection Groups
		- Entity selector
			- hovered
			- held
		- ids.overridden_viewed
		- Some commands?
			- Well it doesn't matter because we restore correct solvable always
		- ticked entities in fae gui
	- What about selection groups when something is removed?
		- For playtesting, we should probably store the entire view state along with the intercosm.
			- Actually, store only the entity ids as they are subject to alteration during subsequent stepping.
			- Btw grouping is command-deterministic
	- The only thing we need to do to prevent crash is to always check if the entity is alive
		- A cleaner approach is to implement clear_dead_entities to be called after each step
		- This is because there is more cases of usage than there are cases of state
	- mover should be deactivated when?
		- corner case: delete while move?
		- should work anyway and yeah, deactivate it then

- Somehow clear dead entities after each seek in main
	- Communicate it somehow?

- Crash when manually advancing but not when seeking
	- Only in release...
	- also check if the plain advance remembers to set the commands properly


- What do we do if we start recording when some commands can still be redone?
	- Well we could simply move the current history to backup for free...
		- ...instantiate new history without having to clone anything at all...
		- and we dont even have to use force sets now
		- plus we have a clean view in the history gui without further ado
		- what about the first revision?
			- -1 will be "Started playtesting" exactly as with "created project files"
	- Do we save entire history?
		- that might be really some pain


- Implement tree node for the children of commands in history gui


- Check for crashes when we have some audiovisuals but we suddenly switch to an empty workspace


- Replaying and commands
	- Replay commands along the solvable's progress as they were applied.
		- pro: don't have to sanitize undos, they are always applied for the solvables upon which the command was originally executed
		- pro: repros replayable even with commands involved
		- pro: can do funny directing stuff
		- con: much space wasted but who cares
	- Implementation details
		- editor history
			- redo shall only move the solvable forward first if there is such a need;
			- only then apply command once
			- undo 
				- if solvable step at which it was applied matches, simply undo once
				- shall only use the seek_to if the solvable step mismatches


- Some strange bug when seeking while recording

- was_modified exists so that the history is saved when some new commands were added later on but we returned to the current revision
	- we would always set modified flags during record or replay because the line gets really blurry here
	- autosave when:
		- playtesting & playtesting dirty flag set
			- one cannot undirty it at al
		- if not playtesting, the history wants to do so or dirty flag is set
		- having gone back from playtesting to old history (which will yield that it is unset) by discarding
			- as the cosmos is completely different
	- disallow quit and show * when:
		- playtesting & playtesting dirty flag set
		- if not playtesting, at non-empty workspace and unsaved revision
	- set dirty flag when:
		- history's seek to revision requested
		- any mutable editor player op

- Fix autosaving when playtesting
	- Revision number might coincide
		- will it though?
	- no autosave happens when revision is -1

- Make snapshot frequency configurable
	- Actually let's just quickly implement map there

- Implement view early to know what's going on

- Implementation order
	- Default chosen mode
	- Initially there should be none
	- Honestly why not choose it at fill stage?
		- No problem just altering the mode before start...
			- Just as we could do it in the first step

- Re-applying process
	- **CHOSEN SOLUTION:** make commands redoable arbitrary number of times, then force-set revision and re-do
		- most to code, probably cleanest architecturally
			- not that much to code
		- best processing performance and least ways it could go wrong, perhaps
	- **RECONSIDER:** undo everything and then re-do when the folder has been restored
		- least to code, worst performance
			- Will it be that bad?
			- Playtests won't be that long anyway
		- although... we'll need to skip some commands while undoing
			- which is sorta dirty
		- least to store as well
		- actually this will save a lot on space
		- and no need to force set revisions
	- always keep a copy of an un-executed command in a separate history
		- this only avoids the need to force-set revision and code re-redoability which isn't all that important
		- worse space complexity, not much to code, not much clean architecturally

- Player start
	- **CHOSEN SOLUTION**: Store only the essentials
		- Don't share a struct for this, just store them in a new struct explicitly one by one
			- That struct will be what should go into unique ptr
			- Contents:
				- Copy of the intercosm
					- From which we will also be acquiring the initial signi for advancing the mode
				- View elements:
					- Group selection state
					- Selection state
				- rulesets
				- ~~From the player:~~
					- initial mode_state
						- actually, no, because we should always reset it to initial values
				- history?
					- NO! History will be the same, later we will only force-set revision

- Constraints in the playtest
	- we'll need to forbid undoing past the revision when we've started the playtest
		- Or just interrupt on undo?
		- Or just create a started playtest command?
			- That's BS becasue it won't exist after re-application
	- History will be left untouched?
		- Though we can always make it different inside GUI
	- we'll need to forbid filling with new scenes?
		- The only things untouched by the fill are history and current_path
		- Shouldn't matter really
	- Some notification might be useful


- Problem: The results of re-applied commands might turn out completely off if...
	- ...some entities were created in-game before e.g. duplicating some entities and later moving them
		- this is because the generated ids won't correspond
	- **CHOSEN SOLUTION:** Allow all commands on the test workspace, but...
		- ...simply sanitize the re-applied commands to only affect **initially existing entities**
		- E.g. duplicating or mirroring once will work just once
		- If we need it in the future we can add some remapping later
		- Just warn about this in GUI somehow
		- Also remove commands for whom affected entities are none in this model

- Manipulating the folder in place during playtest vs keeping another instance and redirecting
	- **CHOSEN SOLUTION**: We should just std::move around on start and finish.
		- Entry points for the folder might be quite many so that's hard to predict
		- Anyway everything in the folder, incl. history, should be quickly std::movable
			- folder has only 1000 bytes

- Existential commands vs those that depend on them
	- Both need be sanitized anyway


- Editor step
	- A number of times that the editor player has advanced, not a cosmos in particular
	- Because a logical step of the cosmos might be reset on every mode round

- Commands could store the editor step when they were executed...
	- ...so that upon undoing, the editor falls back to the correct solvable state



- Editor player, modes and their replays
	- When the player is started:
		- Store 
		- Alternatively, when the player is restored, rewind
			- ...but this is wacky
	- When the re-applying is requested
		- Restore cosmos
		- Restore vars
		- Restore view elements
		- Force set revision in history without making any changes
		- Sanitize all commands one by one as they are being re-applied
	- Existence of initial signi denotes whether the player is playing
		- Actually, we should keep more than a session there, e.g. the saved selection groups?
	- Store optional of state to be restored


- Storage of pre-defined mode informations inside a map
	- ``.rulesets`` file
		- A map is still functional without modes and modes can always be later specified, so a separate file is justified
		- As flavour ids aren't human readable, makes sense to use binary format for now
	- Each should also have a name, e.g. "test scene", "bomb", "ffa", "tdm", "duel"
		- For easy choosing from the admin console
	- unordered map of ints for each defined profile
		- player stores the current int and a variant of mode instances
		- we std::visit by the mode instance on advancing and then acquire the vars by it
	- Editor view shall keep track of current mode instance?
		- We thought about the player, but we must serialize this per-opened folder.
		- Editor view is suitable because:
			- It is the part of the state that is specific to the editor session, but will never be used during gameplay.
			- This actually implies that the **player should be found within the view**.
			- No problem having player state per-view, just that we'll pause it whenever we switch a tab or quit the editor.
		- What about history of changes made during the mode?
		- Probably player as well.
		- The work won't store any 

- test the ensure handler


- Restructure entropy
	- Let entropy be just a map of players into a struct of respective inputs
		- don't hold multiple maps
		- we'll write manual entropy serialization code anyway


- Allow for recording of multiple entities
	- Procedure
		- Given overwrite flags
			- motions, intents 
		- Given some existing stream of entropies for some player
			- On begin recording in some middle
				- Purge entropies in accordance with the flags
				- In snapshotted player, accumulate
				- Before passing total collected, always clean it on the editor_player side

- Game-mode profiles
	- Each map can specify a "profile" with sensible values for a deathmatch, tdm, etc.
		- At the same time, it will be documenting the proper use of the map
	- Will be useful for a built-in test scene
			

-w It makes little sense for an intercosm to have a built-in collection of game mode definitions...
	- ...at least on the grounds that there will be less files overall.
	- because we will anyway have lots of stuff under gfx/ and sfx/
	- and it introduces some stupid corner cases, right?
- so, most of our headache comes from the fact that we have hard time drawing the fine line responsibilities of...
	- ...what is necessary to define in the map itself, e.g. spawn points or flavours
	- ...what should be configurable per every session of a game mode, e.g. roundtime.
	- this dichotomy, once voiced, pretty much speaks for itself.
	- basically, if some game mode properties need to be defined in relation to what is in the map, 
		- then yes, have it in the definition file and configurable in the editor.
		- still can be changed and customized, just saying we'll make UI for this in editor and save/load ops.
	- things like round time, c4 time, they can be specified completely regardless of the intercosm contents.
		- thus they should be textual configs, for which we can nevertheless provide some GUIs.

- server will accept an ``std::variant<free_for_all, bomb_defuse...>``
	- each will perform its own logic

- Construction scripts are a different thing and we'll save scripting topics and their determinism for later

- Let heads drop off after death
	- Won't it be better to implement head in terms of a hat?
		- Hat will be separate to a head so let's have it like a body.

	- Change unordered containers to ordered ones in the visible entities
		- Actually, just provide sort inside layers for domains that require ordering
			- e.g. get first bomb
		- sort_layer_wise
			- by entity_id?

- Since sending of entropy will be highly optimized for space, it makes no sense to have augs::container_with_small_size

- Remove the notion of container_with_small_size
	- Interesting concept but we'll just handle it during actual serialization stage

- Determinism fixes
	- Change unordered containers to ordered ones in the mode state
	- Change unordered containers to ordered ones in the entropy



- However it is to the discretion of a mode *how* the creation and removal of players happen...
	- ...it is already outside of their scope *when* they happen.
	- Therefore, each mode shall expose add_player and remove_player functions to be called by literally anybody, anytime.
		- As for serialization, these will be some "choreographic events" inserted between steps, or in a case of a network session, "network event"
	- Similarly, change_var shall not be something that the mode bothers with, especially that the calling logic would be pretty much duplicated.
	- There will still be much use of the messages; e.g. mode_messages::game_completed to determine the result

- Let bubbles simply be particle stream off-shots
	- Might even better parametrize positioning
	- We will still have fine control over the existence
	- Less strain on the server

- game mode property is a part of game mode definition
- game mode definition = all game mode properties

- Well why not simply set the screen sizes in game logic?
	- Because that bound will be very tightly tied to the view state
	- So maybe finish it like this


- Optimize reinference
	- Don't iterate by guids, just process all entities just like you would always process them
		- this will allow us to hold an unordered map of guids

- Container invariant in editor?

- motion_accumulator
	- setups acquire motions and intents
	- they hold the accumulator that spits just a single motion when we are ready to step
		- well... what if the sensitivity changes mid-step
			- that is a massive corner case that isn't really worth considering
			- the only problem is that the corrections for bounds might be off
			- but otherwise it's not a problem even during tweaks really
	- accumulate before applying adjustments, adjust once just the single motion

- Client-side adjustable crosshair sensitivity
	- Note that we will anyway require SOME form of synchronization of client settings, like nickname
	- **CHOSEN SOLUTION:** How about keeping floats in cosmic entropy?
		- The writing and reading of the entropy for network comms will be **contextual**, anyway
		- Notice that determinism won't be broken if we just change sensi during replay or recording, because the final value will always be held in entropy
		- for now we can just keep sending floats and we'll optimize later
		- what do we do about adjusting?
	- Would it hurt to keep sensitivity inside the crosshair component
		- Yes, because we'll need to synchronize not just network comms on changing a local setting, but even local plays
			- which would otherwise not be necessary
		- though we have to share this information if we want to use shorts for communicating motion deltas

- Fixing crosshair to work with different screen sizes
	- For now the bound is hardcoded
	- Adjust it on each motion in main
		- In the end, review all motions and accumulate them to a single message
			- Later handles the problem of compression
		- accumulation shall happen later 

- file operations:
	- Import intercosm from lua
		- Checks for lua files inside a directory and loads them all
		- Implies starting a new project or clearing history, which will anyway imply an almost clear workspace.
			- So always start new.
	- Export project for compatibility
		- ``ProjectName.int.lua``
			- Contains the intercosm and rulesets, all important things.
	- Export mode vars
		- ``ProjectName.rulesets.lua``
	- new project
		- opens new tab and spawns a folder in untitleds directory
	- open folder
		- opens a folder, loads whatever there is
	- save project as
		- choose existing directory
			- ranger can do that 
	- save project
		- if untitled, uses save as instead
		- same as Ctrl+S.
		- writes all visible project files: intercosm and all rulesets visible to the editor. Purges autosave once done.
		- if saving for the first time to a directory, editor may ask
			- Are you sure you want to overwrite 3 file(s)
				x/x.int
				x/x.hyproj
				x/autosave/x.int
			?	
			- (non-existent files and files with 0 size will be excluded)
			- a different popup design will be in order
				- after receiveing dialog's future result, set an optional pending save object
					- thus we constrain asynchronicity as much as possible and escape it as quickly as possible
				- those that have only "okay" can stay as they are

- Determinism problem: if we re-open a project, we necessarily get freshly-inferred state
	- We can simply re-seek, but when?
	- On all instances of load_folder?
		- Just make dummies for command input
			- Shouldn't hurt

- Add references to lua reads accordingly

- Wandering smokes decoration would be cool
	- We have the state ready: continuous_particles

- Game commencing
	- bool should_commence
		- set when all players in the same faction
	- real32 game commencing timer

- check in editor if the saving/opening path is a valid folder?
	- We handle write errors now

- Marks in editor

- use trigger_just_pressed flag to handle automatic guns

- fix intent contextualization is_secondary 

- Let sentience hold input flags for actions, instead of a gun having a trigger
	- Then only process guns that are held
	- This way we don't have to worry about resetting triggers ever
	- We acquire sentience in the gun system anyway

- Make flying grenades bullet bodies so that we are accurate

	- What to change in physics on throw
		- Bullet: false->true
		- Filter
		- Body updated, colliders refiltered
		- perform transfer should infer the body as well, but not from scratch
	- Let's make it first
		- We'll have a nice entry point in the missile system
	- When to unset the sender of a thrown item?
		- When it first collides with something under velocity that is lower than the required bound for damage?
			- For now it's not of much importance but yeah, it's a good candidate
			- Kills can still be misattributed, e.g. by initiating an explosion
		- We would effectively need to iterate all melee weapons to see if the velocity has cooled down already
			- We could always make it a processing list in the future
		- We might need this for other properties as well, e.g. for setting a filter from a flying item to a small dynamic body
			- And for changing the bullet body type

- asset ids in bomb mode vars widget

- Always throw the primary weapon (and highlight it) if holding melee weapons
	- Throw both!

- fix RMB
- fix png reading


- Windows fixes
	- fix freetype errors

- Fix throws so that they also register hit entities
	- Just add a cooldown for hitting
	- We may remove the oldest entries

	
- Special sentience material vs sentience impact sound specifier
	- Makes little sense to make it a material because a collision might or might not signify damage

- Throwing the knives
	- missile system responsibility
	- pass through held item


- Windows back-port
	- use non-multisampling fb configs on Windows
		- proven to improve performance twofold, on linux
- In game GUI, implement a flag to always choose a weapon on Q instead of holstering

	- Only when there is no weapon allow holster, e.g. when this is the only weapon

- research building with clang on windows?

	- Disallow flavour instantiation when there is no image specified

	- Flipping of editor selection
		- Shift+V, Shift+H

- Commandize "Add player" and "Restart" in editor

- Crash on opening the editor due to snapshots being empty again... when trying to re-step for determinism

- Planting the bomb crashes due to:
	- accessing transform when there is no collider
	- because of cache being destroyed when rigid body is inferred after colliders
	- so either we fix the order of inferences or always infer colliders in the rigid body inferrer 
		- prefer this until there is another problem, for an easy fix

- Add bubbles for poseidon

- Refactor: use damage_definition for damage messages so we don't have to rewrite

- Add inertia for the target rotation when we hit an obstacle
	- Copy the lerp code from interpolation as it was mostly correct

- Checks for animation bounds in melee system

- Chosen solution: let direct attachment offset calculate the knife position so that the entity is always positioned where it is seen.
	- Additionally, manually query once every time the animation frame changes.
		- The change in animation frame will properly be detected by the melee system.

- Setting fixtures vs querying knife fixtures on each step
	- entity-based approach to positioning
		- Then the direct attachment offset must calculate the offsets, I guess.
		- Problem: The hit trigger won't exactly correspond to the knife's shape as we will take convex hulls
		- pro: we can still base hits on a significant constant size vector
		- pro: don't have to fool around with triggers, just accept begin contact events
		- con: there might be some physics glitches since engine will try to push away the collider from the knife fixture
			- or actually it might be even more realistic
		- con: we have to reinfer the melee's colliders every time an animation frame changes.
			but only the melee's colliders, not the physical body.
	- query on each step of in_action
		- pro: finer control
			- over what exactly? Probably only the crosshair.
				- but we can implement a clause for this
					- OR EVEN FREEZE ROTATION FOR THE DURATION OF THE HIT!
						- that's right! that will also prevent cheats where we make a full rotation just to hit something.
		- con: trace-particles won't work out of the box
		- con: trace-sounds won't work out of the box
			- this is big
		- con: knife is always hittable where it was initially standing, instead of where it is in the screen
			- actually this makes our clashes quite complex because we'd have to perform manually n^2 search

- Problem: If we simply reset fighter state on switching weapons, it might be used to shorten cooldowns
	- **Solution**: set_cooldown sets a max of current cooldown and the currently held weapon's cooldown
		- always set it during item manipulation
	- If we leave state as it is, on returning to a melee weapon we can still see remnants of the returning animation
		- This isn't much of a problem

- Always pre-set the cooldown values **and don't use the maximal values of the held weapon** during processing

- And still process cooldowns even if we don't have the conditions for a melee attack in the first place
	- So we're not getting glitches later

- Melee filter: flying bullet

- On clash, apply shake and impact of the attack's damage info, always set to twice the cooldown
	- Though we need to play returned animations

- Melee combat
	- melee_fighter component
		- so that we don't have to hold this state per each melee weapon
	- Returning animation
		- Don't. Simply specify a target rotation to which the character is to be rotated
			- We might even set a slower rotating speed for a while
		- If we hit a solid obstacle, we always return with the animation reversed
		- Do we want another animation for returning when the hit was complete?
			- If we don't want it, we can always set the same animation
	- communicating animation offsets
		- We need information from the animation itself
			- Actually we do hold animations in logical assets so we are all set
	- A melee attack cannot be interrupted, except for solid obstacles and when a collision of two attacks occurs
	- Attack collisions
		- When hurt triggers of two or more players touch, they are pushed away opposite to their facing

- Context help
	- Enums corresponding to text files
- document the offset picker

- disallow throw during a melee move
	- because someone could write a script that always initiates a throw whenever a hit occurs
	- handle drop in the same way
	- actually allow only if it is in ready state
	- simply wait for cooldown?
		- though it is always reset on changing conditions
	- always allow only on is_ready
		- and process cooldowns if only one hand has melee and other has not?

- send knives flying the other way when dropped due to a knockout

- define music as always longer than x and always hearable

- roundsounds and warmup themes
	- we could introduce a separate file for ruleset viewables
		- simply in mode GUI, use a flag for the unpathed/pathed asset widget that determines which viewables to write to
		- alternatively, since viewables are never too big, simply always have viewables separate from the intercosm
			- good for skinning the intercosm without ever having to modify it
	- it would be nice if the sound system could synchronize music against the current arena state
		- though just warmup. no need to synchronize roundsounds which are short by nature
			- specifying the warmup theme sound
				- chosen solution: flavour
					- best performance
					- low flexibility
						- customization without altering the intercosm would involve changing the viewable definition
						- might be necessary to introduce another warmup sound decoration entity flavour
					- lowest coding complexity
					- also lowest design/explaining complexity, not just coding
						- we might be dead before the need for a more complex solution arises
					- a bool for whether the music position should be synced?
						- or a float with seconds tolerance
							- although that would preferably be the client space setting
				- sound effect input, spawn entity with unique sound
					- 'might' incur performance problems?
					- best flexibility
					- moderate coding complexity
					- still two solutions: 
						- a new entity type for unique sound, or
							- even worse compilation times...
						- add a component unique_sound_effect to the existing type
							- memory overhead
				- sound effect input, sound system infers information from the mode
					- best flexibility
					- best performance
					- worst coding complexity - greatest coupling
		- instead of having to create entities? what if we make that functionality cosmos-specific?
		- there could be a music entity that plays globally
		- it would also be nice to have themes in rulesets?
			- well we anyway have viewables definitions in the intercosm so...
			- in practice every alteration in roundsounds will require a different map 
				- and later a rebase, wtf
				- rebases won't be possible with binary files

- adjust doppler factors of bullet trace sounds

- giving money to connected players?
	- always give initial money
	- someone disconnects during warmup - his problem

- Host/connect window
	- non-commanding window with property widgets and a start button
		- since these settings will be critical, we'll hand-write the controls
	- always save to config lua on connect/host press

- Don't use settings_gui.cpp for client/server defaults: simply edit these in connect/host dialogs and save on confirmation
- replace queues with a single optional for item purchase, let us not allow more than a single purchase per step

- watch out for pending_events invalidation if we manually (dis)connect on message callback
	- shouldn't happen though because the loop itself only calls callback for (dis)connections

- general mode command list
	- don't make a namespace for them
	
- custom factory in separate file to not mess up formatting
	- simply cast int to type index in type_in_list_id
	- type_list with all message types
	- then dispatch
	- then we dont need the enums whatsoever and write a simple wrapper in net adapter for creating messages passing the type once

- dont use guids in the bomb mode, its pointless
	- entity guids are pointless overall for now

- put defs for net serialization in hpp, not much to be gained really from a separate cpp file, and we'll have simplicity

- Pre-serialized messages

	- research the completion checking of messages in yojimbo
		- we can use HasMessagesToSend
			- actually not really because its hidden behind a private interface...
		- Use the reference count, simply AcquireMessage on being posted
			- this is even more flexible since it works even if we post more messages after this one
		- perhaps by message id or a changed field value inside the message object?

- commandize add_player and remove_player
	- problem: we might be unable to deterministically predict the outcome
		- what? actually, add player command will always succeed as long as there is space, which we can easily predict
		- we won't predict auto-assignments but that's not really important
		- also if we need predictability, wwe can do these once per step
	- remember that we need to re-add players every time that a mode type is changed

- We need to decide on the server software.
	- focus on creating a minimal api class 
		- whatever we plug there, whether dedicated serv instance or player hosted,
			- we'll just pull our own message format from there
			- We can as well add NAT punchthrough later
				- Games like soldat don't even have it
				- we'll read the guide on raknet even if we're not going to use it as it is comprehensive
	- what we need from the network
		- for early beta and later,
			- reliable delivery for initial stuff + chat
			- redundant delivery for inputs
				- we have to revive the reliable channel
			- perhaps compression?
		- for later stages,
			- masterserver + nat
		- for dedicated servers,
			- secure connections
	- we'll remove enet shortly
	- netcode.io + reliable.io
		- pro: we'll learn more along the way and have more control over packets
	- libyojimbo
		- pro: will probably work right away
		- it's actually for dedicated servers, not player hosted servers
- network_adapters
	- has usings for server/client types
	- global functions to abstract functionality?
		- actually just use a class wrapper and a getter for the specific object so that we know where we're getting specific

- Let someone on spectator when they are connected even if they're downloading initial solvable
	- Though before they git clone the repos
		- Not before we set up the masterserver.
			- That is because clients will know about the map to be downloaded only through the masterserver that will expose the details of all of its servers.

- Try to fix mysterious crash on Windows by replacing std::vector with constant_size_vector wherever possible
	- It appears that it still happens on audiovisual post solve
		- when maybe a ring or a thunder is added?
		- these can easily be const-vectorized
			- have some separate view_container_sizes.h

- Preserialized messages vs keeping structs in these messages
	- Preserialized
		- Pro: Ad-hoc serialization at the site of posting so less risk of inconsistency
			- This is a big deal because MANY desync issues might be circumvented
			- example problem: 
				- a step information is posted for a client.
				- we want to compress it by giving client ids instead of entity ids to associate each set of inputs with a client.
				- however, we can't predict when will the message finally serialize
		- Con: Increased bandwidth
			- though just 2 bytes
				- Actually, this is insignificant, because:
					- the bandwidth bottlenek will be the server's upload
					- and in the worst case it won't even amount to a single player continuously moving his mouse
					- and compare this to the header sizes
		- Pro: Faster estimation
		- Pro: Easier interaction with the server as we don't have to cast the void* pointer
			- Should be negligible?
		- Note that changing the way we write the messages won't really be that much since we will anyway read somehow from these bytes
	- Keeping the struct
		- Has to actually hold the data structure, e.g. a map
		- has to operate on a dirty context pointer instead of our own serialization routine
			- e.g. when the message ultimately serializes for sending the client ids on the server might be different than what will happen at the client

- ESC for closing buy menu and team selection

- Instead of adding a flag to reinfer, 
  can't client simply reinfer any time that a player is added to the mode?
- dont use audiovisual callbacks for re-predicting

- properly remove player on disconect
	- problem: we only have one disconnect event per step
	- simply wait with adding the player until the concerned slot is free
		- we anyway have a moment that there is no mode player id for an already existing client, e.g. before sending of initial state

- yojimbo_assert( length < buffer_size - 1 );
	- report a bug?

- Static linking

- It would be nice if the server_setup could only accept ready structs and was not concerned with messages being preserialized, 
	and serialization in general
	- similarly as it receives "connection event" message
	- let each message declare using payload_type
	- for reading, the payload type would be created on the stack in the adapter, and then constructed by serializators from a separate header
		- server would receive a ready reference to a struct
	- it would be best if the server_setup controlled whether the payload type is to be on the stack or if it's ready somewhere else
		- read_into
		- instead of a lambda, have a template function handle_message which accepts a payload type as its argument
			- then pass *this to the server advance
			- the callback also accepts a callback for reading into so we control if the payload type enters stack or somewhere else
				- if we decided that the yojimbo messages should hold the payload directly, the server adapter can call std::move in its lambda passed to the message callback
		- what about structs that its not comfortable to mash into a single struct but still need to be read?
			- e.g. initial solvable has multiple big objects in a single message
				- mode solvable
				- cosm solvable
				- server vars
			- do we send them separately?
				- feels clean but possibly lost opportunity for compression
					- although these things won't compress much better in conjunction
				- we also get more latency due to separation of blocks
			- read_payload accepts several arguments for specific types, we pass the message type as the type
				- Indeed, it is us who decide whether to pass more arguments
				- we can add a type list as a payload type
				- this kind of thing will happen on the client though

- Whether to keep worst-case storage in the preserialized messages or not is another problem
	- I guess tlsf will be efficient here

- It might be hard to properly send the initial mode state while avoiding desync
	- sending solvable on init is also scalable for longer matches
	- Rounds might be restarted arbitrarily due to pre-solve logic
		- e.g. game commencing
	- and when that happens the cosmos solvable is already altered

- Optimize for bandwidth later. 
	- For proof of concept, it will be enough to brute-force-write the inputs with our own serializers.

- Connection helper interface?
	- so that we have same interface for client and server and dont repeat ourselves
	- we don't have much of these public funcs, though
	- and it would probably complicate matters needlessly
		- even pro yojimbo doesnt to that

- Notice that serialization of mode entropy will be a lot more complicated on the server
	- So, we will have completely separate funcs for read and write
	- We might later write player entropies from the one that posts the most messages
		- we will check the bounds for the message anyways and drop later entropies for now
	- E.g. we will calculate client ids by looking for them in the modes

- Properly send initial state on connection
	- Serialize right away and hold std::vector<std::byte> in the message structure
		- In particular, dont create deep clones of the solvable or entropies 
		- That's because serialization will be a lot faster than deep clones
		- And we anyway have to serialize this so that's one step less

- Client FSM
	- PENDING_WELCOME 
	- RECEIVING_INITIAL_STATE
		- after receiving initial state, we might want to send inputs that happened as a block as well
	- IN_GAME
	- Last time of valid activity
		- Kick if AFK
		- AFK timer can already be a server setting


- The built-in player of the server
	- Always at id -1
		- mode always allocates maxmodeplayers + 1 in its array
	- The server setup will thus hold entire intercosm

- Let's just create a proof of concept using yojimbo, don't focus on creating a minimal augs interface
	- When it works, we'll gradually abstract it away and see if it still works
- if we don't send any message at all for when a step has no entropy,
	- we CANNOT possibly advance the referential cosmos in time
		- which might become problematic once we have to simulate it forward several seconds
	- so we should always send at least empty step message


- concept: server session
	- mode-agnostic sever vars
		- server_vars
			- perhaps they should not AT ALL influence the deterministic state itself
				- problematically, logic_speed_mult does influence this state by influencing the requested delta
				- so does the tickrate though
					- although that should only generate entropy
			- max players? 
			- okay why not simply have both the tickrate and the audio speed in arena mode ruleset?
				- simplest solution, not pure semantically but still architecturally somewhat
				- we'll still have *some* server vars
			- basically they should only change how entropy-generating code behaves and the timing code
			- timing info
				- tickrate
				- snap audio speed to simulation speed
		- these actually can be called vars, why not
	- server step != mode step?
		- server step tied to tickrate?
			- the cosmos and the mode anyway don't know about the rates, just of a delta to advance
	- server step will also be passed to the packed
		- only ever zeroed-out on hard server resets OR when there are zero players?
		- server step does not have to correlate to the cosmos step at all
			- in fact they will most of the time be different due to rounds being reset
			- so it's fine to always zero-out the server sequence when there are no players
	- we can make the server always have the built-in functionality to play locally with a character
		- for local plays, it won't be much of an overhead
			- it doesn't even have to be used for pure local setups
			- can be used for the editor playtesting to have full test experience?
				- actually even then it's not required
		- menu guis won't be processed anyway
		- for now we don't need a barebones server
			- we'll test it when the time comes
			- for barebones server, we can simply compile out the window, opengl and openal, even lodepng, leaving us with no-ops at all
				- server won't rely on image/sound information anyway, this will be the task of the mapper to once create a proper map
				- we can also simply set a flag for whether we want audiovisual processing
				- also completely compile out the vws 

- Entropy optimization
	- Chosen solution
		- call translate_payload once on the server per step
		- manually multicast this message to connected in-game clients
		- it is identical for every client
		- if the client's num accepted entropies != 1, send a separate message BEFORE sending the step
			- in the best case it won't be sent at all so we might even save this byte
		- the client holds std::optional<uint8_t> last_num_accepted;
			- zeroes it out every time a step message arrives
			- sets it every time a prestep_client_context arrives
			
	- It's best if the server preserializes the entropy instead of serializing it each time for every client
	- Well we could send the same allocated step message to all the clients
		- That will be the most performant and probably easiest
		- "broadcast_payload"?
			- so we'll use the same buffer for all clients
			- we could always attach a lambda that determines whether this client needs it
		- we will thus need to put commands_accepted in a separate message
		- well what if we send num_entropies_accepted via unreliable channel? 
			- e.g. you could send total messages accepted
	- thus we need to send bytes as the payload


- Leave a detailed net graph for later
	- Just print some basic network usage

- Let rockets collide

- Fix crash on kartezjan's komputer

- Fix ammo indicator for rl

- rsa returns 0 for chamber mag if chamber loaded and the flag specified

- Reloading the rocket launcher
	- We certainly want to take advantage of the chambering mechanism to add a sequence to the animation
	- Problem: if we make a gun chamber magazine...
		- ...we'll effectively allow two rocket rounds to be loaded into the launcher
			- is this bad?
				- that would make the launcher pretty op
				- technically we could compensate with fire rate bound
				- alternatively we could make a clause in can_contain or query_containment_result
	- Problem: If the gun chamber is physical, we'll be able to mount directly to it
		- we can make a clause for the gun chamber slot type as there's no weapon that'll ever need to directly mount to the chamber
			- we can always allow unmounting from the chamber for the cool effect
				- although that is not important for now
		- we'll only ever mount to the gun chamber magazine & then perform chambering
	- Note there is no item to be unmounted, like it is the case with empty mags
		- Will we skip the gtm animation?
			- Anyways only the rifle has distinct animations for both types

- fix pickup sparkles missing at stacking pickup

- If the charges don't all fit into inventory, allow to pick as many charges from the ground as possible
	- Or do we force them to be picked by a free hand?
		- Perhaps!

- rocket launchers
	- For the fire trace, we could take the fire muzzle shot particle effect, and repeat it with high frequency
	- Simply add explosive invariant to the plain missile?
		- Do we have any use for the hand fuse there?
			- nah, it's for beeping, arming and defusing
		- will support cascade explosions out of the box
	- add sounds later

- The rocket item entity
	- Should be a normal cartridge


- let team_choice/item_purchase be a variant
	- we can already serialize variants anyway

- autodrop cue sound threshold mult 

- don't turn off menus on buying replenishables

- Gather all items to the left of the hotbar

- double the impulse in hand fuse's release explosive
	- because we no longer apply standard impulse, that was sorta stupid

- fix nades not dropping when switching weapons
	- If not in a slot type where it was originally armed, autodrop
	- we don't have any prediction issues here
	-  in-gui
		- pro: cheaper to calculate 
		- con: how do we request a throw?
			- We sure as hell dont want another type of entropy just for this corner case
		- we can check in logic if the drop is concerning an armed explosive
			- and allow a standard impulse flag to determine force since it will also be used for proper releasing of the explosive
				- If it is a standard impulse (e.g. due to a request), just put in a throw impulse
				- the standard release explosive will 
	- in-game
	 	- pro: throws handled easily
		- con: calculated for each armed explosive
		- con: needs character settings
			- eventually, we'll need it

- Make a flag for autodropping of magazines
	- And implement this gui-side
	- "Autodrop magazines of dropped weapons"

- drop all mags on dropping a weapon
- bind h to buy menu
	- or something else

- let q also look for the bomb if the hotbar empty
- Fix build without the editor

- fixbomb planting duration
- fix esc for that damn team selection
- dont show defusing progress if youre tt

- Automatically pick the bought weapon
	- currently it is held into eq if something different is held, I think


- Trim reach inside component please
	- the marker type probably as well as it is annoying
- Preffix the single client entropy with a byte.
	- The first bit signifies whether it is cosmic or mode
		- we will only allow one type per step to increase number of message types possible within remaining 7 bits
	- Remaining 7 bits will signify whether there's data for:
		- cast spell
		- wielding
		- intents
		- mouse
		- etc.
		- for mode: item purchase, team selection etc.


- General client processing loop
	- Always check and add a mode player if the client is not yet in-game
		- Handles restarts automatically
	- if we do it per step, there will be a delay for the state to catch up, but:
		- State is simpler
		- We don't have to worry about malicious sizes
		- We don't have to worry about message being too large due to all these nicknames
		- Con: we might waste more bits/bytes overall but that's amortized over time

- Will yojimbo handle 128hz?
	- yeah, it even did handle 144hz!
	- We can send packets once every second tick, so at 64hz

- even test scene mode could specify its fog of war size for scaling?
- fix HUD when scaled

- bool adjust_zoom_to_available_fog_of_war_size
	- true by default
	- some clarification in settings gui

- Perhaps fix crosshair bounds to be zoom-invariant?
	- And the crosshair

- Should parti/sound existence systems translate gunshot messages or shall we leave it to the post solve?
	- What if something is destructed?
	- Actually during gunshot nothing except cartridge is, and it is not taken into consideration

- Honestly we should simply always be defensive about existence of entities in audiovisual post solve
	- and only sometimes clear caches so that they don't get too big
		- advances should just iterate caches with erase_if
	- note that unsafe handle access is only there wherever cosmos reference is, so follow the cosmos

- BETTER CLIENTSIDE PREDICTION
	- Battle events shall always be played referentially
		- Not sure if always but that will be decided contextually anyway
	- We might want to predict sounds depending on the effect type
		- E.g. tie FoTA effects to the referential
		- But UWoTA to the predicted as there is a delay
	- We might want to give a context info for the start struct
		- This would hold the perpetrator id
	- Collision sounds
		- For now - predict all except collisions with remote players and their items
			- If a collider has a capability of a remote player - play in the past
			- otherwie always predict
				- Even throws should be fine
					- although some sounds might be unplayed if e.g. something is thrown closely to the wall
					- we could introduce a recent owner struct in item
						- if the recent owner is a remote player, play in the past
	- clashes
		- never predictable
	- deaths
		- never predictable
	- arena mode gui
		- doesnt play any effects AFAIK
			- Only the tick sound
			- and plays the welcome message
			- these are negligible
	- When to clear the dead entities from the audiovisual state?
		- technically can the audiovisual state ever assume that the entities are alive?
		- Standard post cleanup basically ONLY clears the dead entities
		- the need to cleanup will be properly detected
		- Okay the post-solve should perform cleanup depending on the cosmos of the step it was passed, not just once
		 	- the need for cleanup will be properly detected then for the client switching between referential/predicted
	- making the interface easier by just modifying message queues and calling solve 
	- the test setups and so will implicitly realize the prediction_input::offline by never modify any messages
	- we also have to advance the sounds
		- problem: for which cosmos do we advance, referential or predicted?
		- we'd need to keep track which effect is which
	- we always advance against predicted, so if something starts existing in referential suddenly...
		- ...it will automatically exist in predicted right away
		- problem: if we repredict after an abrupt change, the shot bullets could get some different ids
			- so the predicted orbital caches of bullets that we shot will be deleted and will never come back
				- well wasnt this a problem even without any additional logic?
				- this is an edge case though, won't be much visible
	- actually av-postsolve shouldnt do any deleting and unsafe 
		- so maybe there's no point clearing it every post-solve, just once before it happens

	- av-advance always handles important interruptions, e.g. of sounds
	- Okay, for now just always advance against the viewed cosmos
	- and post solve against two
	- how do we ensure no dead entities pop up somewhere?
		- the safest bet would be to always sanitize at post-solve
		- post solve always (at least it should):
			- checks whether the cosmos has changed
				- if it did, full sanitize
			- at post solve the entities queued for deletion were not deleted yet
				- okay what about those deleted without msg?
		- post cleanup as well if any deletion occured
			- if any entities got deleted
				- if any did, full sanitize
		- advance should be suited for sudden deletions
		- afaik sound system's update properties check all handles
    - lets just always clear dead entities to be on the safe side
	- **Idea:** two audiovisual states?
		- Ultimately, we always draw the predicted cosmos	
			- problem: if we advance some particles against the referential cosmos, we'll get bad homings and it might look bad
				- so it is not a silver bullet
		- problem: we'll need to draw from two audiovisual systems
	- **Idea:** can't we just always advance against the predicted cosmos?
		- Problem: some advancements might not necessarily be predictable, e.g. later we might want to handle gun engines referentially
		- if we had two avstates we could advance one against predicted and one against the referential
	- We should calculate interpolation and the visual systems only against the predicted cosmos?
	- **LOW PRIO:** okay, the audiovisual system might still need to advance some things against the referential, and some things against the predicted
		- its a completely different problem from post solve though
		- for now it is not so critical to confirm these, it's just variation in gun engines is all
			- and these are quite predictable anyways

- Actually, perhaps we should never predict our own death, even the impact.
	- That is because once we're dead we don't care anymore
	- and we should always assume we are alive so that we can still apply some valid inputs which could otherwise be impossible
	- some step settings for that

- If we detect inconsistency of state

- dont call gui post solve for both predicted and referential
- less music volume by default, e.g. 0.3
- separate dash logic for reuse in different contexts

- cooldown the dash on switching weapons to balance insta switching

- rename bound to bound

- ao44 should be explosive
	- or just burst fire
  - electric triad should explode on hit

- more mana, less shield absorption
	- perhaps only reduce damage instead of absorbing
	- so we can still cast some things
- autocast bought spells, especially shield
- bug: remove screen resolution advantage

- when inverse or exponent model, use max_distance for reference distance as it works somewhat this way then

- remove ensure false from 174 in server setup

- autodrop of magazines sometimes doesnt work...
	- it was perhaps due to post solving gui twice

- make crosshair go to the very bounds of screen even when zoomed
	- it's a problem even with equal aspect ratio

- left/right hands fire buttons are swapped, hard to decide what to press

- autofill all magazines to full on start of each round

- autofill magazines on start of round

- balance
  - UWoTA should have less delays and could propel
  - grenades could explode even more in all directions
  - more mana points at start
  - less recoil for pro90 and more damage, maybe more velocity
  - less delay for automatic shotgun, more damaging pellets, higher velocity
  - mostly it is the low velocity that makes weapons underpowered
  - dont do live logging for the first time as the performance will be shit 
- dont show bomb planting hud when you are ct
- let fota have a high impact

	- Knife wielding
		- Won't 1 and 1+2 be enough?
	- Maybe automatic knife throwing?
		- mouse wheel up would be good
			- in this case the throw button itself would become useless
		- Would always return to the last picked weapon
		- though that will work out of the box

- Hotkeys for:
	- Grenade wielding
		- A settings flag for whether to arm automatically and release on button release

- Autohide empty weapons during akimbo?

- Problem of predictability of autodrops?
	- It anyway begins with an unpredictable entropy of a drop
		- Except when a nade is thrown
		- Though the post solving input might arrive in time?
		- The release of explosive is unpredictable though

- autoswitch on weapon drop
	- actually maybe no because the walk makes us faster
	- this can be done gui-side when post solving the drops


- autopick previous weapon after throwing a nade
	- not really, let us be fast
	- but only if no more items left
- autoswitch on empty ammo

- learn about -frounding-math -fsignaling-nans
- replace std::shuffle with something simpler or just

- test libm linkage with our single cpp files
	- and maybe float stress testing as well

- Stress testing the floating point determinism
	- Launch a test only once on startup when no cache folder is yet detected
	- if it fails, show a warning window to contact the devs immediately
	- Procedure
		- Spawn 3*cores threads (to see what happens under heavy loads and if the registers might)


- RNG consistency
	- we'll need a portable uniform float distribution

- fix bounds editing of position variations in light

- don't use degrees_between in fish logic
	- just use dots and crosses
	- look for more occurences of this

## About this: we just flush the queues

## Rest

- Why does the client play bomb being dropped after having received the initial state?
	- Does it simulate something?
	- it simulates the predicted world in the meantime.

- freeze the player that has just joined
- A desync still happens locally
	- Setup round is probably the trigger
	- It once happened on step 700+ possibly triggered by a disconnection?

- Once we could reproduce a desync right after a connection, so is it a problem with connection init?

- We totally need a hash comparison so we know right away what causes a desync

- Why don't we draw bomb progress gui in arena?
	- but we do for editor!


- Transfer requests for drops generated by game gui might occur when the mags don't exist already as well.
	- So sanity checking there is in order as well.
	- We must also check if the entity is authorized to perform this transfer.

- Wields might have occured for items that do not exist already.
	- Especially after they get deleted
	- Wields must be checked if targets belong to the entity
- bug: after knockout, some weapons remain tied to the player
	- this only happens in online play
	- The client might have sent some wielding requests after its death, 
		- because it falsely predicted that it would stay alive.
		- I remember this occuring even during recordings in editor
	- The wielding and transfers are not really that checked for validity, I think.

- fix ctrl+v on windows
- check if editor now launches in windows release build (not relwithdebinfo)


- Apparently, we still get desyncs even with a reproducible float library.
	- check out these reinferences because maybe desyncs are due to them
		- stress testing needed
- check if performance problems persisted in older versions, those played with kartezjan and before repro math
- prefer low dampings
- maybe allow running for longer as it is a running heavy game
- reinferences should NOT occur at all during play because we are cloning	
- 2 seconds longer freeze time

- increase max predicted from 130 to more 

- Fix prediction of arena gui

- (Shuncio) some possible crash before start of the next round

- forbid arming grenades when frozen
	- happens only for mouse hotkeys

- walking around many shells slows down so it's probably due to logic


- arena gui predicts wrong
	- draw deaths only for referential, i think it might be the other way around
	- problem is, it draws based on state not on events?
		- but the referential state for the bomb mode should properly have the kill
- knives should not be thrown if wheelup is pressed during freeze
	- simply dont allow to request throw during freeze
	- fetch wheelup in the arena gui also in case you buy something after the freeze

- increase range of secondary throw

- fix smoke particles becoming bubbles as we still have a good repro
	- perhaps effect ids remain the same but content is different

- the personal depo still gets closed sometimes
- prevent trigger pull sound during chambering
- disable buying in warmup
	- or just 0 money
		- but we can still get awards so better to just disable it 

- can probably buy when dead
- the problem still persists with weapons being glued to someone
- freeze someone on connection if they connect

- fix those dangling particle streams as they may give away positions 

- separate slot for the bomb, OVER_SHOULDER
	- It would always be drawn over the backpack.
	- We might decrease the bomb density a little so that the bomber isn't too heavy.


- server_setup
- client_setup

- always holster the weapon first if reloading

- Watch out if fpset does not cause performance problems

- knockouter is dead if someone disconnects after having shot and that bullet kills another player

- balance exaltation, perhaps higher mana cost - 100?
- lower price for interference nade
- lessen shake even more
- save last player nickname used to connect

- fix effects of shot weapons that were just thrown 

- consider always autoloading the shotgun?
	- NO! Simply add WAAAY more recoil and kickback for shots if we are reloading
		- Pro: nice effect for the last shot as well
		- we can also go pretty wild since pathological weapons like ELON HRL won't ever be shot during reloading

- maybe increase max predicted commands

- reinfer when loading ends?
	- it should be handled by the transfer though wtf

- check if steam engine works when item is thrown

- decrease smg density
- increase price of exaltation and of uwota
- decrease price of fota?
- fix behaviour of container component in editor

- we still have problem with reconnection even after re-initializing the net lib
	- even on linux

-balance
	- m4 damage: 33
	- ak damage: 38


- spectators
	- preferably only in the client setup
		- this state should be held in arena gui 
			- arena_spectator_gui
	- limit for spectating dead bodies
	- spectated player identified by mode_player_id
		- it can become unapplicable due to many reasons
			- misprediction
			- after death delay passed
			- uninitialized (e.g. when entering the game for the first time and can't spawn yet)
		- whenever it becomes unapplicable, search the next to apply automatically  
			- based on the last saved ranking
			- if uninitialized, whatever comes first
			- last score could be a good rank
			- followed by the nickname
		- store last rank to preserve some kind of ordering

- bug: crash in openal when creating new tab in editor after having played in the first tab

- fix knives not giving rewards

- Introduce gradual slowdown when stepping on an unfriendly surface
	- amount_ms
		- const vs linear? we'll test
	- damn, we need to reimport

- Optional variation for movement of moths, in movement path system
- Layer for flying organisms

- fix cosine of angle being wrongly compared causing a bug in movement path system

- A command to add official flavours to the map
	- Would not add existing ones
	- Check if one exists before adding?
	- well, what about to_image_id? we'd have to worry about creating everything in order
	- Eventually, we will make a division between flavours of weapons and the rest of the map
		- Although do we really want to do this architecturally?
	- update_official_content command
		- Creates a test intercosm on the side
		- When a test cosmos is fully created, it adds flavours to the current cosmos if cannot find one with the same name
		- Adding just allocates and copies the contents
		- After everything is added
			- In the destination cosmos, introspects all flavours for identificators (easy as they are trivial plus 
			- Looks for other, portable/reliable means of identifications like names
			- existing id can be converted back to an enum and this to a name
		- This could be done automatically whenever a map is loaded, actually
		- can look for introspection code that checks where an image is used, maybe reuse it

- correct those footstep sounds, maybe make the distance model linear?
- fix szturm sound being the same as bilmer

- vent hit sound is too silent

- multithread light vis calculation

- organism_cache
	- do we do inferrers for both areas and organisms? 
	- plan 1: just infer organisms
		- infer all just iterates over all organisms like any other cache
		- what if an organism area suddenly changes geometry?
			- we can have a refresh function for when that happens
			- and reinfer everything when altering this in editor?
				- well then just use that refresh function
			- "recalc grid"
	- Infer both organisms and areas
		- Need to account for a moment when an editor alters any of these
- New settings tab: Performance
	- A flag: simulate_small_organisms_during_net_reconciliation
- Connectivity issues
	- Random disconnects
		- Safely allocate even 20mb per client
		- increase timeout in yojimbo and others?
	- Check why "server shutting down" would occur

- Start to version these maps
	- just ignore histories and views etc 

- fish optimization: a simple grid
	- we can also put an upper bound of the processed neighbors, e.g. process 3 at most
	- linear time

- fix naming in raycast profiler (fow vs light vs visibility)

- Definitely increase the reward for win
	- Maybe increase reward for loss just a little?

- Decrease flashbang price to around 700$
- Increase bomb planting time by half a second
- Flag: show spectator in past

- At some point we might want to give different jitter buffer length values to 60tick and 128tick

- Always aim with the hand that has an armed explosive
	- If both have, prioritize primary
	- Also if we have a scheduled throw, aim with the first melee weapon

- We should somehow re-adjust the amount of predicted steps
	- Sometimes even if the latency is low, the amount of predicted steps is high
	- Do we properly take squashing into account?
	- Ah, it will most probably depend on the jitter buffer length of the server and the current value

- log if client does not have the map that's on the server

- Just append the accumulated motions to the current crosshair position in illuminated rendering
	- That way we have a smooth crosshair even under low tickrates
	- And we always have shots directly to where we were aiming, unless we move a bit after the shot but before the step

- Fix cue being too loud
	- Or just fix it for bilmer

- random shutdowns might be to lag spikes as well
- well, just increase message buffer sizes?
- If a player wears a bomb, don't show a separate bomb on top of it. Just show the bomb icon next to the arrow icon and color it appropriately.

- Make indicator image a square anyway so that it rotates nicely
- Raycast against a smaller rectangle to smoothly transition from left to top edge etc

- show defusing character like we show character with the bomb
	- copy the defuser icon from scoreboard
- Show only allies' colors by deafult, like in CS GO 

- Assign colors to players like in CS GO
	- Then show colors on radar
	- And also indicators should just be colored arrows without nicknames
		- You can always check the nickname after pressing TAB
	- The indicators will also have the distance in meters and callout name under it
	- When somebody dies the indicator turns to a cross
	- Show a red pistol indicator when a shot is heard

- mark backpack as already owned if we already own a backpack of the opposing faction

- re-assign colors to players with fallback color when someone lefts

- fallback for when colors are exhausted
	- No means of identification in-world when colors are exhausted
		- 4 first characters of nickname as secondary_text
	- We anyway have to have the arrows to show the direction off-screen
	- It would be stupid to disable all colors if we sometimes exceed the player count by just one
	- So let's have a fallback color, light gray?

- two layers for callouts so we can stack one on top of another

- Fix black textures in team selection
- Why does client not get notified of disconnection properly?
	- It has two server addresses so it waits for disconnection from the other one
	- solved with kicks

- Always set filtering to linear when rendering avatars
- Show a smaller avatar preview next to the big preview

- Preview the avatar in client start gui, give it a separate texture and use some special imgui logic to show it?
- why hotkey for team selection works late

- Avatars
- max: 80x80, 64k
	- resize automatically in-game

- ping

- Dealt:
- Given:
- Death screen
	- [Avatar] Billan
		- Killed you with their Bilmer2000
		- Damage given: 32 in 2 hits
			- Count real contribution in damage owners, not mana decreased when shield owned
		- Damage taken: 4 hits, 384 applied damage, 100 HP lost, 0 PE lost.
		- Damage given: 2 hits, 180 applied damage, 0 HP lost, 289 PE lost.

- Allow to spectate others right away after death
	- Always hide when viewing us
	- Let LPM and RPM navigate through players
		- However let them work only a second after dying 
	- Let the default delay for friendlies be a second
		- Dont show stats for friendlies
	- Let the spectator gui track time instead of having these variables in bomb mode ruleset
	- Just spectate right away and always hide the spectator gui overlay when showing our body
		- The death screen is enough of an indicator

- Allow spectator to watch everything
	- Automatically assign a player to watch on connect
	- For choosing response, don't show the spectator text overlay if we're choosing teams
	- Allow to choose spectator team only if you have an alive character (or for the first time, when you're already connected)
		- This will prevent griefing when someone goes to spectate the opposing team right after their death

Looks like the game crashed since last time.
Consider sending developers the log file located at:
%x

If you experience repeated crashes, 
you might try to reset all your settings,
which can be done from Settings->Reset all settings to factory default.

- at least set config to main menu on crashing
	- We could compare write dates of exit success with ensure failed to see if we can take the local config

- Writeout the config before starting a server process from the menu

- Actually just hide the less important notifications of other players to not make spamming easier

- Game notification variant
	- Subject
	- Faction choice result
	- Daedalus, you're changing teams too fast.
	- Daedalus, you can't go to spectator if you're not alive.
	- Some would be hijacked by chat gui and others by the main notification system

	
- "X connected" notifiers in chat, in gray
- chat notifier: X has joined the Resistance/Metropolis
- chat notifier: X is now a spectator
	- Except when connected for the first time?

- Keeping an integrated server option has the advantage that it will force you to keep the client code clean


- Host a server menu
	- Radio box
		- Integrated server instance
		- Dedicated server instance
	- Might still make sense to have integrated server instances for slower computers
		- Will have to refactor the chat, though, and avatar handling
			- Separation of concerns might be useful here as our client setup class is a god object
	- What about machine admin player id?
		- if it is equal to max clients, we might miss it accidentally if we use max_incoming_connections_v in loops
		- if it is equal to max clients - 1, it wastes a single connection slot?
	- For checking whether a dedicated server instance is running, we can keep a lockfile named after the port
- Controls
- Test assists after fixing the damage owner logic

- see if bilmer and vindicator have better intervals at 64hz


- fix shot cooldown progress visible behind walls
- add markers for allies that are off-screen
	- simply clamp the nicknames to bounds and maybe add an arrow
		- then just clamp to a smaller bound to account for an arrow


- Communicate with the server somehow from localhost?

- And fix polish characters in chat breaking stuff on Windows
- Add imgui popup whenever regenerating neons or atlas, but only in the main menu

- fix in-game area indicators wrongly drawn when zoomed out
	- they can still be drawn zoomed out in-game if the monitor is smaller than 1920x1080 for example

	- Contextual
		- Press "E" to defuse the bomb.
		- Stay still while defusing the bomb.
		- Press "T" to buy items.
		- Press "C" to pull out the bomb.
		- Stay still while planting the bomb.
		- You are a Spectator. Press "O" to select a team.
	- Define "Context"
		- The entire arena handle, I suppose?
		- Should then arena gui mixin manage it?
			- I guess...
	- Draw somewhere around where the death summary shows
		- Nothing should appear once we're dead
- fix those knife offsets on character

- Fix annoying corners on cyberaqua

- Port cyberaqua again from the same lua files to avoid re-importing and loss of accuracy


- Callout markers
	- Separate trees
	- names
		- catwalk
		- pump
		- generator
		- toxic a/toxic b
		- horse
		- turns
		- helipad
		- radars
		- cache
		- t vent
		- computers
		- ct computers
		- mid vent
		- green mid
		- office
		- brains
		- connector 
		- knight (konik)
		- stairs
		- turns
		- long b
		- ninja
		- fern
		- b lobby
		- b gate
		- neck
		- gate
		- fork
		- glass
		- hole
		- mixer
		- secret (instead of knight)
		- b vent
		- mid vent
		- back vent
- Strange bug when reloading the rocket launcher
	- It might be due to ids changing when un-stacking to put a charge in the hand
		- Then the id of the wielded stack (of one rocket) is different from id of the original stack
			- and the reloading gets interrupted, and the rocket gets stacked into the original stack
				- ad infinitum
	- Solution: Simply never interrupt the reloading when all other candidates EQUALLY good or worse
- Fix gradobicie price

- show ping on integrated server
- ao44 finish load sound is unused!!
	- let's test it thoroughly on linux

- hrtf still crashes on windows
- HRTF does not crash on Windows but it does not work anyway
	AL lib: (EE) Failed to set Stereo, got 5.1 Surround (Rear) instead
	AL lib: (EE) Failed to set 44100hz, got 48000hz instead

- Increase buy time
- How do we even kick bots on the server?
- Show last HRTF status below Enable HRTF

- editor too should have a loading notice, at least in editing mode
- fix these project-relative paths for neons at last, so that stamps dont get versioned
	- Reason: When picking a path, ranger and others probably return the full path, instead of relative
	- However, a server or a client, when given just "arenas/de_cyberaqua", detect the adjacent neons files to be custom neons
	- Actually this way is maybe better because we don't have to regenerate them a million times

- (flag) Treat holstered as a separate setup for Q


- Release triggers on match summary and halftime
- for variation, don't switch teams after match summary
- re-freeze when match starts and someone respawns at warmup

- hide indicators for yourself
- Zero out the crosshair displacement when spectating
- Fix spectator for integrated server

- Clear particle systems when switching setups


- Flags for serialization of specific entity types through the network
	- We could really stop serializing sprite decorations and static physics bodies
	- We need some kind of contexts for these serializers
- Do the same optimization for objects that haven't changed their positions, e.g. crates

- Check wielding logic

- Per-character settings to store within entropy
	- Implementation
		- We still have one bit free inside the player entropy so we could dedicate it to 
		- Or have a separate message and better optimize the mouse
		- To have some struct coherency, we might mve out the settings of basic_player_commands to a separate map
			- as it won't sit well with additions and it will be sorta left out of the client commands serialization procedure
			- still it will be in cosmic entropy which is not sent through network but reconstructed instead
	- They could be stored inside the per-character entropy itself
	- The cosmos-ready entropy could hold it, but the packed one would simply take it from the settings
	- Any change to settings to these would go along the client_commands channel
		- Note that the server has to broadcast these settings deterministically
		- public settings might be a part of client meta
	- pros
		- We can save on the mouse movements
		- We could compress them even down to two bytes per movement in the most optimistic case
		- just one bit to determine the range
	- what if there is no map entry for the current character entropy?
		- Then in theory there should be no need to use these settings, e.g. there will be no applications of force or crosshair movements
		- We could always cache some last observed values

- Clientside, avatar update received
	- Non-existent session id arrives
		- case: The player is already gone here
			- Then the session id will be less than next mode session id
		- case: The player is not yet added
			- Add it to pending meta updates
	- existent session id arrives
		- all fine and dandy, assign to meta
	- add player arrives
		- clear the current meta for that player
		- check the queue for early ids
			- if no matching, wait until new one arrives

- Clientside, chat kick received
	- Non-existent session id arrives
		- case: The player is already gone here
			- Then the session id will be less than next mode session id as it was added already
		- case: The player is not yet added, either latency or was kicked before being added
			- Add it to pending chats
			- the case for being kicked before can be signified with session id = 0

- It sorta can't be helped because we want a deterministic, uninterrupted stream of inputs and asynchroonusly send less important, complementary info
	- Each add should then pop and process the pending commands for this session id
		- remove does not have to do anything as the add player clears relevant fields
	- if only we could confirm on the server that the step at which a player was added, was indeed confirmed...
		- although that's more to screw on the server and more latency introducted?



- Completely wrongly assigned metas
	- Solution: session index
	- EMPIREFAN had fortesq's avatar
	- The integrated host appeared to have some unique ping to all other players even though it should be zero
	- FortesQ didn't see his own chat messages, only after reconnecting
	- Since viewing logic does not write the state, we can parallelize it heavily with thread pools
	- integrated server client might be screwing up order of ping values and avatars?
	- Explanation for pings
		- Net stats could have arrived before the client has appeared on the server
		- later, the stats for integrated client were never pushed to the end of vector of pings (because we were taking only connected)
		- So it was frozen to some ping value of the client when they arrived
	- The avatars might'be been left uncleaned, simply
		- Both on the client and on the integrated server, billan saw orh's face in empirefan's avatar even though there was no avatar
		- We only rebuild when a NEW one is sent, but we never clean them and if someone does not have an avatar no message arrives
			- so it cannot be told if the avatar has not yet arrived or if there is none
			- CLIENT-SIDE, we need to clean it somehow, though the avatar might arrive before or after we get a notice that someone's connected

- object_command<T>
	- if has_object
		- dispatch
			- if static then object::perform(payload)
			- if not then object->perform(payload)
	- set_uniform will have a payload variant for simplicity
	- command and command_payload
	- set_uniform
	- set_uniform_payload payload

- Let textures and all construct normally
	- The standard "set as current" interface simply pushes a command to a renderer
	- Similarly with shaders

- minimize use of get_uniform_location
- Minimize reads from GPU by precaching the uniform locations

- The atlas in arena server might be screwed the same way that avatar preview screws everything
	- probably wrong texture is set after teximage2d

- Implement guards for cursor clipping functions
- set_cursor_clipping(bool enabled)

- minimize access to window whenever game loops
	- somehow maybe watch for changes to screen size?
	- or for now just acquire the screen rect once
- look for a event-based way to keep track of screen size
- Separate thread for render queue
	- We'll just queue commands for now
	- The finalizers can happen in the rendering thread
	- We can still query for the need of new atlases in the main/logic thread
	- Perhaps the same with openal if hrtf proves to decrease performance

- research ping behaviour under low performance
	- Our 140-150 rtt matched against the server-reported ping in scoreboard
		- so it was probably a case of low fps at kartezjan's

- performance settings
	- explo sparkles
	- explo smokes
	- explo thunders

- check shake for bomb explosion

- Measure performance of dead particle removal 
	- how does it measure against integration itself?
		- looks like for 10k particles it's just around 0.02 ms so it's really not worth parallelizing
		- perhaps just launch in a separate job but even that might be an overkill

- drawing particles depends on completion of multiple jobs. we might want to have a more complex system here
	- Simpler solution: let a single posted job do:
		- create n jobs for the particles
		- join in the effort until last completed?
		- the same thread posts a rendering job
	- however, after processing 2k particles, we can post a job to render them right away
		- why not make a job do both? process a range of particles and then draw the same range!
	- we'll need to avoid races on output particle triangle vector
		- so let's preallocate a vector with the target number of particles

- To avoid repeated allocations, we can have a pool of triangle vectors
	- Although for now I guess the driver does a similar thing so let's forget about it

- setting: split particle jobs once every n particles

- Properly show timing values minus the time waiting for the other thread
- Render thread might begin helping when there are still no jobs posted
	- So it should become a legit worker thread instead

- Allow audio to be processed only once per logic frame

- Problem: if we want audio thread to help processing tasks, we must somehow guarantee that it will exit once it has new jobs
	- Help until there are no audio frames
		- which is also cool if audio thread only has jobs per a logic frame 

- Since we're going to clear the task pool we could just use a vector instead of a queue
	- And we would not need to hold a mutex for getting the next task
	- Though I guess it's premature
- We'll somehow remove the multithreading facility completely from the visibility system
	- We'll take control
	- Actually, we won't even need the responses because the tasks will fill the data themselves
	- FoW for our character will be a separate job
	- Somehow pass a pointer to the target triangle buffer to fill. If it's null
	- Light system simply does a delayed call_triangles(dedicated_buffer_category::VISIBILITY, i);

- Caling dedicated buffers in a delayed manner
	- dedicated_buffer_type::FLYING_NUMBERS
	- FoW buffer
		- similarly, call_triangles(dedicated_buffer_type::FOW)
	- We can later give dedicated buffers to the render layers similarly, just a separate enum type and enum array
		- this only has the disadvantage that it will incur more drawcalls so maybe we could do it later

- We either queue these particles or synchronously advance thunders and explosions
	- Drawing can happen separately anyway
	- We would anyway need to synchronously copy the particles so lets just go with synchronised advance

- Problem: exploding rings and thunders spawn particles upon advance
- Remember to post the heaviest tasks as the first, so audiovisual advancements go last probably

- WARNING! Interpolation should be run before parallelizing other systems because it might affect transforms!

- If the render thread sleeps, we should be able to use it to speed up the frame preparation in jobs system
	- Otherwise we have to be conservative in the number of threads by one
	- Why not just post render jobs?
	- game thread could post all the jobs, incl. rendering
	- process_jobs_until(game waiting_already)
		- or just until empty since the thread pool will be emptied of jobs by the end of each frame
		- same for audio?
			- if audio has nothing to do, it can help in producing the next frame as well
	- game thread itself will process jobs

- Allow delayed invocation of renderers
	- We could at least make light drawing a separate job 
		- with its separate renderer

- Backend accepts renderer so that it has access to dedicated buffers
	- Can post a drawcall with an enum instead of pointer for a delayed call?

- Task dependency graph
	- The particles don't have to be completed before illuminated rendering - just preallocate the buffers so that the pointer values don't change
	- Separate renderer for game gui
	- Separate renderer for post-game gui

	all jobs:
		advance_thunders();
		advance_exploding_rings();
		advance_flying_numbers();
		advance_wandering_pixels();

		launch_particle_jobs();
		audio_job();

		perform_illuminated_rendering();
			- could be further parallelized

			Depends:
			-> advance_thunders()
			-> advance_exploding_rings()
			-> advance_flying_numbers()
			-> advance_wandering_pixels()

		actually why not just preallocate and draw after advancement to avoid dependencies at all?
		we could later similarly jobify tasks per layer

		game_gui_job();
		post_game_gui();

		show_developer_details()

		join all

		

- The multithreaded model
	- The main thread
		- Problem: augs::window might be needed in some other areas, not just in opengl thread
			- e.g. spawning dialogs - can't be static because it needs a hwnd
		- Always performs the simulation logic
		- Dispatches work for viewing
		- Initializes the workers
			- Audio command thread (sleeps on condition variable)
			- Rendering command thread (sleeps on condition variable)
			- We don't need thread pools for these yet as these can be standalone threads
			- We can later poolize the work for all remaining viewing logic
	- (second concept) The main thread
		- We could make it actually hold all window related stuff
		- The rendering thread
		- Also initializes the workers
		- Spawns the game thread or just a job?
		- game_frame_buffer
			- this will be swapped
			- it will contain the local entropy vector as well
		- logic won't sleep between intervals, it will just continuously calculate new audiovisual state and wait for frames
		- what about imgui drawcalls
			- the audiovisual step produces a new one each time
	
- Thread pools
	- The game loop
		- Setup posts a job to complete n steps
	- Interpolation, particles, sound, drawing layers, all these can be calculated independently
		- Actually they all need to join before illuminated rendering, or just before their respective layers
	- Game gui too can be drawn independently
	- Start with std::async?
		- Actually some nice interface for completion could come in handy

- Things to think about after introducing commandized renderer
	- Textures after creation are bound but this fact is not registered in settable_as_current
		- This may just be ok
	- watch out if clearing (moving from) the triangle vector does not screw things up
	- void clear_special_vertex_data(); was really never used?


- We badly badly need some non-serialized metadata components for simplicity
	- what for
		- high performance fields would otherwise fuck up padding (pointers, e.g. vectors)
		- faster processing for some data (interpolation)
		- avoid serialization of irrelevant garbage serialized pointlessly
	- pool name: synchronized_arrays
		- can be empty
	- manually specify synchronized arrays
	- pool vectors are forced to be nontrivial so we can make an empty overload for interpolation and physical caches
	- Could be held in pool 
	- Same for all entities anyway
		- Actually not, some might want to have a cached colliders connection
	- Keep same index as real objects

- Fixes to audio
	- seek_to crash
		- would it be too due to bad meta?
	- crash on exit
		- terminating with uncaught exception of type std::__3::system_error: thread::join failed: Invalid argument
		- fixed: we had twice the quit
	- sounds disappearing randomly
		- we were reading wrong variations
	- heap-after-free usage in generic sound cache init
		- might have been due to those variations too
	- too many sources?

- Some lights are skipped

- Mouse sometimes freezes movement when lpm or rpm is pressed 
	- happens only when we change windows

- Let's do the delegation thing later, I think openal is thread safe and just creating and loading new sound buffers should not at all alter existing ptrs

- Since it will happen rarily and it'll be good for performance to just pass naked pointers,
	- let's simply synchronize with the audio thread whenever we make a sound buffer transaction
	- but let the audio thread do the actual loading
		- and we'll compare the frame numbers to determine if it's complete, instead of having futures?

- what if we delegate a loading job to the audio thread as a generic std::function command?
	- doesn't it happen asynchronously anyway now?
	- we just have to either finish processing ascertain that
	- we need to ascertain that we only ever post existing pointers

- test if lowpass_gainhf >= really cuts it
	- just test flashbang basically

- WHILE SOUND BUFFERS ARE LOADING, remember to finish and halt the execution of sound system  to avoid data races, e.g. due to meta writing
	- A command for bufferdata?
	- Actually if we anyway commandize creation of bufferdata, why not commandize erasure of buffers and thus also commandize clear_sources_playing?
		- this way we don't even need completion mutex/notification

- so many uses of getters...
	- perhaps let's really use these metas.
		- POINTLESS!! We'd have to pass all transforms every time.

- We can really easily approximate if the sound source should already be over with playing,
	- but we can always return a vector of sound sources that ended, from the audio backend itself, every sound frame

- Let's have a unified means of source identification. Having a template for this will bloat our code.
	- same list of free ints can be used for short/continuous cache construction
	- Let's keep renderer backend simple.
	- to clear_sources_playing, we can pass a deleted buffer id instead of assets::sound_id
	- and additionally to sound system, but it will only clear caches, not stop/destroy sources

- both short sounds and continuous can be faded
	- we need to include means of identification
	- however we don't exactly need to request deletion of short sounds because we'll pool them

- Hey, audio thread can actually be just a thread pool with a single worker.
	- Bonus: we get a free interface for waiting for tasks to finish.
	- we also get a free interface for task submission
	- con: we should watch out to never add another thread to the work
	- con: we have to be able to actuall add tasks
	- it would be good if we avoided re-allocating the entire sound command buffer every time
	- a fixed-size circular buffer would be in order I guess
	- we can entirely avoid the sound frame generation step upon overflow
	- okay so maybe let's have a separate thread so that we know exactly what's happening

- well, still, how do we remove the short sound caches even if we just map an incremented id?
	- do we really have to estimate? what about randomly changed speed in editor?
	- Actually we don't have to erase short sounds because we'll just cycle them.
	- Logic-side, we'll delete continuous sounds the moment that handles are dead or the logical conditions change.
	- Sound backend-side, we'll delete the sounds once they stop playing.
		- Logic-side, we can make issue special commands upon deletion so eventually these sounds stop indeed playing.
		- We actually make them start fading.
		- Or we can make a command to deallocate a source.

- other caches can be mapped to entities. the unordered map logic will happen on a background thread so it shouldn't be a concern

- clear sources playing
	- however it cannot conflict with the audio thread.
	- we could put a mutex on audio thread whenever it is processing
	- "finish()"
		- well, would work just like gl
		- audio would just notify a cv when there are no more tasks
	- called synchronously from the game thread
	- called rarely so doesn't have to be fast

- What if we could extract some game metadata to that separate thread?
	- pretty hard. still probably need some means of identification
- Number pool.
	- For now, if we run out of sound sources (unlikely because we have a ton), we can just prevent a new sound from allocating.
	- How do we determine in sound system if the sound source has stopped playing?
		- Actually we probably don't have to since it's only about erasing sound sources to save memory and we'll anyway pool
		- We won't ever do erase_if anyway
		- However, we need some means of freeing up the identificators
		- We can approximately calculate the time of ending in sound system

- Can't we use augs::pool for this?

- A pool with an internal sound system counter seems like the best option for identification
	- We'll measure if mere existence of sound sources makes any difference

- a specialized command for handling flash_noise_source

- Use pointers for sound source identification?
	- We could also "request" new sound source to be put into a unique ptr

- unique pointer of int, not audio source
	- actually same problem as with audio source as pointer
	- by the time audio backend decides to communicate the id for us the sound system might have deleted the sound and the memory address becomes invalid

- Referential is assigned from but it its interpolation is never advanced
	- thus the glitch
	- so we need to preserve interpolation somehow

- FIX INTERPOLATION
	- Before assigning to predicted, make a temporary backup consisting of all interpolation arrays + ONLY indirection ids
	- Resimulate prediction
	- Only then, restore interpolation values from the backup to what can be restored
		- Technically if entities were allocated in different order, all bets are off, so we could as well just rewrite the arrays without remapping
			- We might try this first
		- Destruction of entities is sorta predictable
	- So probably no "assign_interpolation" function at all, we'll do it on our own

- Interpolation is still fucked up with ping, it never was like it before
- Audio parallelization
	- We might just use a concurrentqueue to push audio jobs, preferably with a set maximum
- for sound and rendering, use readerwriterqueue repository?

- Fix the sudden increase in upload rate when someone timeouts 
	- After some time, we could send initial state instead of inputs
	- Aggressively disconnect?

- Optimize bomb explosion - remove unnecessary sounds
	- It doesn't even sound that good

- Sleep multiplier for server

- Test polish characters on Windows again

- Test on windows what's gonna happen if we try to host a server twice on the same port, also on linux

- Test avatars on windows, especially the png filename filter

- Fix that clipboard on all platforms at last

- In host a server menu option, simply spawn a separate dedicated server process and use locally created client_setup to connect
	- Something to check if the servers is already on

- Still, if we are able to always connect and catch up with inputs, it should never disconnect if we resync mid-game
	- Using debug mode to simulate a slow computer does not reproduce the issue

- Theory: weak clients don't catch up with the steps they have to re-simulate after desyncing
	- Why would they then be able to do so on connection init?

- Treat holster as last setup flag?

- friendlies off-screen indicators
	- meters instead of pixels, because of varying levels of zoom

- Stress test connection under desyncs
- Fix spectator to show the past

- actually make two rulesets for server and playtesting
	- perhaps do so from test scene content and replace on update official content

- flipping of the other hand in akimbo?

- minimally buff pro90 over zamieć: perhaps increase the round's impact a little?

- The rocket launcher still glitches during reloading, sometimes
	- watch the footage from chrzan

- For chambering, we might use reloading animation but without flip 


- Fix that bug again!!!!!
	- It depends on the player order
- buff datum gun
- nerf amplifier arm

- Absolute paths are remembered under Windows

- Try to reimplement that swap buffers without UB and so that it doesn't crash on windows

- Balance grenades
	- +0.1ms delay
	- A little increase in price

- less delay for uwota
- fix triad missing explosions

- galilea can't be hidden. We must decrease it's space occupied drastically
	- possibly even mags

- do_server_vars
	- both in settings gui cpp and in rcon
	- just simply apply & save buttons along with revert
	- these changes are too important
	- we can easily make snapshots of vars applied

- Re-transmission of server_vars as a universal rcon feature
	- Handles map changing
		- How do we load arena?
		- Do we deterministically re-insert all clients?
			- Probably 

- Maybe allow map changing through rcon so that we could switch to this other map?

- Check if there's no room for error in those connection tokens

- You have released the cursor. To control the crosshair, press Tilde again.

- jpg/jpeg file support for avatars
	- stb_image

- Another detailed state machine for repeatable steps
	- "REMOVING NEW HYPERSOMNIA" etc.
	- There will just be a loop until message box is pressed, though synchronous would be best

- Missing openssl dlls on Windows

- Download archive to cache/Hypersomnia-for-Windows

Say the game is in ``C:\hyp`` folder, so e.g. the executable is at ``C:\hyp\Hypersomnia.exe``

Old client does:
- Unpack the new, downloaded game to a new folder: ``C:\hyp\NEW``
- Copy the player preferences from ``C:\hyp\user\config.lua`` to ``C:\hyp\NEW\user\config.lua``
- Launch ``C:\hyp\NEW\Hypersomnia.exe --upgrade`` and exit

New client does:
- Recognize the ``--upgrade`` flag that tells us that the following actions are required 
- Move everything inside ``C:\hyp`` (except ``C:\hyp\NEW``) to a new folder: ``C:\hyp\OLD``
- Copy everything from inside ``C:\hyp\NEW`` into ``C:\hyp``
- Launch ``C:\hyp\Hypersomnia.exe`` and exit

The new client (in-place) does:
- If it exists, remove ``C:\hyp\NEW``
- If it exists, remove ``C:\hyp\OLD``


Advantages:
- If the new executable fails to launch, the old executable with all of its files remain completely untouched
- The old files are removed only at the final step as a cleanup procedure (that is not even necessary for the game to function), when everything's confirmed to work
- If shit hits the fan in the middle and something crashes, the user is left with nicely named NEW and OLD folders
- We don't have to play with any paths outside of the game's tree, to which we may have no permissions

- Restarting the app
	- https://www.gamedev.net/forums/topic/580762-how-can-i-restart-my-c-application/
	- https://stackoverflow.com/questions/40022586/is-it-possible-to-restart-a-program-from-inside-a-program

- Build file server
	- corner cases
		- What if linux build and windows build finish at the same time?
			- either linux or windows build will stamp the build and it should be the one that takes longest
				- so, windows
		- What if two travis builds finish at the same time?
			- they will have stamping disabled so nothing bad will happen
		- Two appveyor builds can't finish at the same time because we've checked rolling builds
		- What if windows build completes before linux?
			- The app will detect a new version but won't be able to download the binary because it is not yet there
			- We have to handle this case anyway
	- security
		- secure api key comparison
		- are the denied file candidates left dangling in memory?
	- Appveyor could hold a release_notes.txt artifact
	- But we need retention so appveyor is a no-go 
	- Folders
		- builds/
			- latest
			- 1.0.1323
				- windows
					- Hypersomnia.7z
					- commit_hash.txt
				- linux
				- Hypersomnia-x64-windows.7z
				- Hypersomnia-x64-linux.7z
				- Hypersomnia-linux-x64.7z

				- hypersomnia-1.0.9430-linux.7z
				- hypersomnia-1.0.9430-win64.zip
					
				- Hypersomnia-for-Linux.tar.gz
				- Hypersomnia-for-Windows.zip

				- commit_hash.txt
					- for client-side version check
				- whats_new.txt
					- Optionally

			- 1.0.4383
			- latest

- versions built on travis are fucked up
- Add update links to settings
- Rename Advanced to Debug

- Disable HRTF by default if concurrent threads <= 4
	- just remove the entry from default config and calculate it in main inside canon config lambda
	
- Store avatar in user
- Upgrade the server!!!! It has the old map

- Rcon server vars application doesn't seem to work
	- They do but not on the integrated server, and probably rightly so

- Changing map on the integrated server doesn't re-add the admin player
	 - Actually it doesn't even change the map

- Avatars are lost after changing the map
- Fix mouse behaviour in fullscreen

- We should check if the spectator gui is ready for sudden complete changes to the sampled cosmos
	- if some entities aren't actually dead for example

- Write autoupdater so we don't have to save those avatar images in temporary
	- We'll keep them in cache, just autoupdater will preserve some config values
	- We might keep the entire config local lua intact, just per-push specify what config values to overwrite
	- Maybe just preserve the local config and untitled editor works
		- though with this everything could be preserved
		- generated file checking should be pretty safe
		- we have an option to get factory defaults
		- one could easily delete the user_config.lua
		- kay what if we only store the modified settings in the user_config.lua?
		- like a delta
		- we'll then always have defaults updated
		- fix these imgui layouts though, "choose team" shouldn't be resized as well as maybe shop
			- we'll just give a sensible multiplier
			- close to fullscreen for shop


- Automatic updater?
	- Would also solve the problem with cache?

- Hold user avatar/nickname/other identity in appdata folder
	- so that we don't have to specify each time a new version comes out

- Port fy_minilab
	- though change "invariants" to "invariants_state" and same with components
- Upgrade appveyor
	- Things to update for windows
		- Appveyor script: llvm 8 when the visual studio 2019 is ready to take the new filesystem
			- We had differing versions, the one on our computer is newer so it builds everything
		- augs::date_time constructor for file write type, used to display last write times of lua meta files, although that's low priority

- let GUI sound volume just be the sound effect volume
- Master volume

- Watch out to properly recognize if connect or replay was pressed
	- just clearing if non-replay button was pressed

- is_replaying: just query the state from snapshotted player
- Let's just do the recording first

- We'll create a simple code for recording, but we might offload replays to snapshotted_player

- Fix mouse inputs being applied during demo replay

- Spectator inputs don't work
	- and make it navigable without closing the demoui
- Add spectated player's avatar to spectator overlay

- client_setup replay (re-visited)
	- let packets from server be saved on the message level
	- I think adapter should just have if elses since client setup logic is way more important
		- negligible overhead really
	- Design the file so that we periodically append

- client_setup replay
	- for now we can have entire client setups as snapshots
	- adapter can have a pointer
	- we'll be able to re-use snapshotted player
	- instead of copy assignment, just destroy and do placement new
	- just move the client setup snapshopts
	- other approach
		- we might want to have a demo_replay_setup	
			- it can manipulate client setup's local collected entropies (won't allow window inputs)
			- it can also set the messages to appear in the given step
		- i know it's gonna introduce setup boilerplate but it's worth it for control and separation
		- it can have a vector of client setups
		- note that spectator gui should work in the demo replay setup
			- we could make it derive arena mixin with a singular arena gui and not at all care about the arena guis found in client setup snapshots
		- since we want to be able to spectate all players, note that only our client will possess the actual prediction information


- Remember about proper locking when flushing with async
	- e.g. protect when_last_flushed_demo and unflushed_demo_steps
- Benefits of demos
	- Deterministic repros
	- Can record without performance hit
	- Can later record in highest quality only the highlights
	- Fun moments will never be lost

- We can automatically record demos for every server session
	- Demos could just be network messages applied at step x
	- We could pretty much resimulate the entire client setup this way, just without sending messages
	- Also makes it easier to debug
	- Snapshots could prove a little hard but we could just resimulate from the beginning if we want to seek backwards

- Buff bulldup: maybe higher rof?

- Fix path problems with demos on Windows

- Wait with updating maps after changing stats, at least until we make a gameplay teaser
	- That is because we have to record the demo somehow and otherwise the replay would diverge
	- Okay, we can do it now!
- Fix GUI subject for spectator demo replay

- Dedicated server communication
	- Advanced RCON functionality
		- Rcon can download the log
		- Rcon should have a reserved slot to enter in case of emergency
		- RCON password
		- Switch teams
		- Restart
		- A way to view dedicated server stats?
	- Remember to keep the old master rcon password so that basic level rcons cannot change it

- We still have crashes on Windows, even after fixing swap buffers
	- Perhaps it's a compiler bug since with our home-brewn build everything seemed to be fine
	- Although I think it was the same with that mega.nz build


- To avoid transmitting some server-decided seed for the beginning of each round (e.g. to position players around)...
	- ...we can just derive a hash of all inputs from the previous round, or just hash entire cosmos state
	- this way we are unpredictable about it but still deterministic
	- Seed will have to be sent in the beginning anyway, along with the state
	- Some amount of initial information will need to be transmitted anyway
		- Like current players?
		- Isn't this all a matter of sending the bomb mode state?
 check how openal behaves on Windows when abruptly stopping sounds


- Game events log and chat
	- Positioning based on input box window
		- Under the input box window, we can have tabs changeable by ctrl+tab and ctrl+shift+tab
	- Don't show input box window when chat is not active
	- Scroll can be added later
		- If we ever have scrollbar, change range, not coords.
	- Always show n recent commands. 

- Sending large step infos through yojimbo?
	- we probably want to handle it after DM milestone
	- Don't rely on fragmentation

- Chat-level logs
	- server_setup has to expose events somehow
	- can send them really as chat messages to all the clients
		- we also need to redirect it to the server player
- Step configuration for the cosmos
	- Whether to process deaths, e.g. to never predict them on the client
	- Whether to post audiovisual messages, always false for the server
- Client-side
	- When initial state is received, wait for the first entropy
	- When it arrives, simply begin queuing inputs localy
	- The server only sends a "client input accepted" byte in response
		- when this happens, peel off the oldest input from out queue
	- always re-simulate all inputs in the queue
		- we don't have to calc some difference, this will happen naturally
	- how does this approach scale when the effective latency suddenly decreases?
		- so a server suddenly gets a burst of packets from the client
			- if we unpack them evenly into steps, we don't decrease the effective latency
		- since some steps were missed, now we have to squash inputs
			- server sends a number of how many inputs were squashed?
		- don't worry, by definition squashing will only occur in high-jitter environments
			- squashed entropies should still preserve important behaviour
			- e.g. you won't be left with a gun that shoots even though you've released the button already
			- magnitude of movements might be malformed so we'll have a hitch, though nicely interpolated
	- how does this approach scale when the effective latency suddenly increases?
		- client just doesn't peel off inputs for a while from its queue
	- to avoid squashing as much as possible we can have a server-side jitter buffer for clients
		- though I remember we didn't have some good experience with it
	- if the client input was not accepted, still peel off the back queue!
		- simply treat it as a misprediction!
		- well, this sucks, because we can possibly miss some important inputs like a button press
		- suddenly our player stops moving!

- Notes on assymetric latency
	- Effectively, the client always shows AHEAD the server time by 
	- Therefore it is the client->server latency that is the most important
		- We should ALWAYS resend the earliest messages since traffic on this side will NOT be the bottleneck
		- so set the delay to 0 on client-side config

- Chosen solution for jitter buffer
	- Handling latency increase and thus, unaccepted client commands 
		- Client adjusts naturally, the same way as in the beginning of the play where latency is assumed to be 0
	- Handling latency decrease and thus, packet bursts
		- Two strategies
			- One, squash on the server 
				- Pro: simpler, so we'll for now go with this strat
				- Pro: gets the client fastest on track
				- Con: slight jerks when this happens
				- Was the same not mentioned with snapping of the ticks?
			- Second, slow down tickrate on the client and thus the rate with which the commands are generated
				- Pro: no jerks when this happens
				- Con: takes more to get the client faster on track

- Server: accepting inputs
	- I guess a simpler jitter buffer implementation could be in order
		- e.g. just keep a vector and a maximum of steps to squash at once?
	- jitter protects from latency increase, squashing from decrease


- If we simply don't predict knockouts, we automatically don't predict the vulnerable win conditions in the mode
	- Other win conditions are based on time so it won't be as bad
- fix arena gui showing "Disconnected"
	- somehow cache the nick or remove the entry?
	- this is cool actually but if someone connects right away it will show his nickname as the victim
		- due to id collision
		- maybe store nickname?

- Probably simply play win and death sounds in accordance with the referential cosmos
	- Will avoid confusion
	- Though will introduce lag
	- If, on the other hand, we want to predict deaths, it would be best if these were death sound entities
		- So that they get interrupted on mis-prediction
	- From what I can see, we already had some lag on the death sounds due to empty beginnings
		- Around 40-80ms
		- And we never noticed
		- So let's just always make death sounds referential


- Admin panel
	- Editor-like server vars tweaker accessible by pressing ESC server-side
		- will have to be commandized properly, just like editor setup's
	- Should we re-use change property command?
		- we'll only need a dummy editor folder struct
			- we could make it more flexible
	
- test O3 with and without flto?
	- Could save us much of the build times for production

- why would a warx fq12 be reloaded?
	- is another mag chosen for it?
	- do we use strict greater inequality to acquire the better mag?

- fix client being unable to reconnect
	- the effects are being made unpredictable due to being thrown
- When appeared
	- So that new servers can easily be found
- remember to shut down the aux server

