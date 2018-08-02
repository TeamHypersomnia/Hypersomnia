

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
	Regenerating texture atlas: cache/gen/atlases/game_world_atlas.bin
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
