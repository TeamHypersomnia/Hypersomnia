---
title: Brainstorm now
tags: [planning]
hide_sidebar: true
permalink: brainstorm_now
summary: That which we are brainstorming at the moment.
---

- What do we make official?
	- Vent sprites certainly because we already have an official vent physical material
	- Maybe some complex-shaped stuff
		- The steel crate
	- Entire garden biome

- First sort out physicals and handle the aquarium spawning at the very end
	- Apparently resizing physical bodies doesn't honor the custom physical collider.
		- Only if it's a box

- Finishing touches
	- We gotta wrap it up
	- Resizing animations

- We can ship editor as soon as we reproduce fy_minilab
	- So people can play in the meantime
		- and I can start redoing other maps

- Filters window
	- Visible
		- (Checkboxes) Backgrounds Physical Foregrounds
	- Selectable
		- (Checkboxes) Backgrounds Physical Foregrounds
	- Let's just add an eye to the toolbar
		- Later optionally "Filtering" to View

- Aquarium - We only really need to think how we'd do it manually and support it in the editor
	- The rest is only about automatized placement, we shouldn't worry about these two simultaneously

- Is our model still incomplete?
	- If we allow specifying custom resources in the aquarium node,
		- we will need to implement the non-physical override for the officials as well
	- Also think where creating organisms fits into this all
		- Are they just in the foreground domain? (esp. the insects must be foreground)
	- We shouldn't worry about duplicate flavours
		- at least the image defs won't be reused
		
- Cover ground neons per-entity?
	- Only foregrounds will need it on a second thought
	- Consider making anything foreground cover the ground neons by default, but I don't know how we'll do it
	- When something needs to cover ground neons per-entity it's usually static
		- So we can do the job with a static neon occluder
			- Although that's PITA when we want to move the entity so maybe let's have per-entity switch after all

- silly idea but until we have a geometry editor we could have default colliders in various shapes
	- e.g. triangle_wood_collider
	- sucks that we can't set material per entity but screw it, we can list all properly, there won't be many of these
		- won't hurt to support it longterm as it will be basically blank texture
	- then we can resize/rotate it as we will which I think already works with physical shapes (e.g. crates can be resized)

- The physical shape editor simply specifies the default shape
	- This shape overrides what would be shown with the standard in-world geometry editor

- Ctrl + Scroll could move nodes up/down
	- We could save it in Edit menu (Move Up in Layers) so we don't forget

- Server playtesting
	- Once we hit clapperboard once, "Play" could automatically refresh the server
	- Let the other player still play once we go back to editing mode
	- Other players can always spawn by default where the host spawns
		- will collide if some two connect at the same time but doesn't matter
	- Then their positions can be preserved when they refresh the map
		- That preservation can be done server side easily because server_setup will just receive a signal to refresh it 
			- and will readd all players to the test mode
	- can show an ip address to copy next to the clapperboard
		- or even copy it automatically on pressing ctrl+p when it's first hosted
		- the clapperboard might actually just be to host the server and play will enter the game
			- when ip shows it will be a nice feedback that the server was hosted

- we could as well have a separate window for things like project-wide playtest settings 
	- even though it will technically be a project setting
	- but won't hurt to have some specific settings separately from inspector i guess
	- Project->Playtesting
		- yeah maybe it'd be good to have a menu for various project settings to indicate they're indeed project-wide
	- there's no mass edit involved here

- Playtesting settings: how do we save them?
	- A separate window?
	- Maybe it should be a project-wide setting? With all the default mouse angles etc, default factions and weapons
		- Yeah probably just save it in json
	- Immersive mode can just be a flag ofc
	- We can have a menu entry for this why not
	- default guns/faction could be customized too
	- default spawn node setting will override the spawning at view-center 
		- it will be a project setting

- Mirroring works differently than flipping for animated sprites, but that's probably because we don't have flip property in dynamic decorations

- Playtesting
	- Alright let's do this, it will be rewarding anyway
	- Separate setup?
		- Hosting a server will require saving the changes first because we'll actually switch to server_setup
		- Normal playtesting can easily do it in-editor as it will use just the test_mode so won't even need to save changes
	- Server: To notify or not to notify
		- Maybe notify by default and only hide notifications for passworded servers?
		- For now it would be good to always see if someone's making a server so let's notify by default
			- Won't really be accidentally 'spammable' as you'll have to save the changes first and maybe fix warnings/meet the conditions for game mode etc

- Json readwrite and ignoring defaults
	- With reads it is really simple, if we read e.g. a physical resource we can set the default before reading directly into it
	- The best default provider would be just the default type
	- We might have a per-type default provider passed to write_json

- The whole idea of overrides is stupid af it'll just complicate our editor arch
	- We'll just have to copy the file and be done with it, atlas will be easily able to recognize it
	- Also we should really have the option to unpack the aquarium into nodes.
		- So that we can do a thing similar to fy minilab
		- If you want non-physical aquarium walls you'll just do it yourself, that's just a template
		- Will autoselect the unpacked nodes

- Uhh can't we force just separate resources?
	- The domain overrides complicate the shit out of creation pipeline
	- We could later have virtual resources

- Note that even if we wanted to make this aquarium manually
	- we'd still need those lab walls with overridden domain

- I'd think of a nicer way to serialize augs::maybe<> in json
	- Either have enabled = true/false inside the struct (in practice these structs are guaranteed to have no member like that because otherwise what would be the point of maybe<>)
	- or at least name it disabled__organism vs organism instead of enabled_organism
		- actually "organism__DISABLED": because we sure as hell won't have capitalized members and it's visible clearly

- We've touched on separate topics here
	- Automatized placement/special prefabs specification - this is solved, we'll have enums and structs per each usecase
	- Domain overrides for nodes
		- we were worried about too many flavours which led us to think about static allocation limits
			- But maybe we shouldn't worry about it for now? We'll have to increase the limit anyway, I think flavors will be smaller
		- Ultimately we could make the render invariant a component so we can have just a single flavor (?)
			- Still would be a waste to make the physics cache dereference it
		- Okay then we should necessarily just lazy-allocate the flavors we need for overrides
		- Only thing left is how we think of organisms here because the physical domains won't specify moving organisms
			- Although insect sprites will always be foreground so I don't get the problem
			- It's just whether we want to make it a separate domain, maybe yes? Organism could be both bg and fg


- A node should have an option to make it non-physical
	- So it can be spawned just as a decoration because sometimes the thin walls might be used standalone physically, or separately over the layout defining colliders
	- Some physical nodes might also be overlaid transparently (?)
		if we determine we want our own physics for an aquarium for example
	- Alright so the aquarium surely should have the option to not create the physical aspect
		- 1) because it might be somewhere off-screen and it'd be a waste of resources
		- 2) because it might be used in a layout-defining context and it might conflict with the walls
	- In legacy editor, we have instances of the same image being used in both physical and non-physical context'
		- Notably lab_wall

- Note that domain could be overridable on a per-node basis
	- This is because physical domain involves strictly more properties than both foreground/background (which are the same)
	- So we could enable this option for nodes of physical resources

- Okay but what are the remaining problems with this prefablike special objects design?
	- If it's area marker, it will not correspond directly to aquarium's physical bounds
		- Not big of a deal but still, especially if aquarium is meant to be layout-defining
		- On the other hand if we make the organism area the actual physical bound it will be confusing that it's larger than the actual bound for fish
			- However most likely it will not be noticeable
			- We'll just have to hide rendering of the true organism bound marker which would be ugly
	- Okay I don't think we have the choice, we just have to accept physical bounds will be larger and go with the first, simplest solution
		- Someone might want to specify different wall sizes anyway so for layout defining aquaria they will have to figure it out on their own 
		
	
	- Other is where do we spec the default resource setup for the aquarium walls glasses etc.
		- The defaults will obviously be official
		- and I think we could technically set them in editor_node_defaults, but maybe we could store and preset them somehow somewhere (?)

- Couldn't we implement "special resources" in terms of "node templates"?
	- The previous "special resources" design was really just a response to ours not having implemented enough node/resource types
		- With all organism entities, particles and organism areas in place, we could make aquarium just an instantiable node group
			- We can still call it prefabs for what it's worth
		- it's just it will spawn an entire layer
	- No no no, we lose all the parametrability we wanted to achieve
		- the deal was that we wanted it to be instantly resizable for example
	- We wouldn't be able to have a simple json type = 'aquarium' with just size and be done with it


- Maybe ask during Q&A if we should indeed reset the tab or not

- Actually map could have in its settings two checkboxes:
	- Include all invisible layers
	- Include all invisible nodes
	- This way we could edit some specific fragments in peace but have them always finally be included
	- I believe this should be off by default as it will only become useful as the map grows really

- Also we have all required special objects for aquarium
	- Well.. except for the organism bound marker

- Now that we're dabbling with special items..
	- Let's spawn all necessary things for a bomsite
	- The map should technically support as many gamemodes as possible
		- Once we have many gamemodes we'll let the map creator select which game modes to initially support (they have to choose at least one, create button will be disabled if there's none)
		- We'll have separate layers for objects pertaining to these gamemodes
		- gamemode_common 
			- buyzones
			- team spawns
		- gamemode_defusal
			- bombsite a
			- bombsite b also to let someone know intuitively there can be more than 1
			- bombsite c too since we already have an icon lol
		- gamemode_ffa
			- neutral spawns

- Also let's similarly have colorizeable particle streams with the initial color in node_defaults

- Consider renaming all "colorize" to "color" in the editor
	- Once we're doing serialization tho

- Once we're remotely close to MVP - EVEN WITHOUT A GEOMETRY EDITOR YET!
	- We should start rebuilding fy_minilab pretty soon
		- This way we reason what is really needed at this point
		- Per-sprite geometry editor will obviously be necessary too but we'll port it
	- We'll need a way to mass export image parameters like neon maps
	- We could make it way easier: just export neon maps as pngs, we don't care about rest of the parameters
		- Except shapes... but we could reapply them, shouldn't be that many
	- We might want already export to json from the old editor
		- We'll import in json in the new editor
	- It's a cool idea - next to a png file there'd be a file.png.json file
		- These would ONLY be defaults
		- So they'd be values after pressing Reset in resource view
		- It would make it extremely easy to port resources from one map to another
	
- By the way let's thoroughly check which officials we want to list
	- We might also want to change some names for accuracy so that it's future-proof
		- Will break maps but only if we wanted to press "Update official content" (in player window) on them

- We can finally implement game atlas culling now that we don't use it in rendering
	- Remember ad hoc won't need culling because we'll just use tiny tiny icons
	- Only the inspector gui will need using ad hoc

- Also add insects and garden sprites to officials, will be handy to have all biomes
	- Maybe it will motivate is to implement culling
		- which will only break the official firearm rendering for now
		- And might break animation rendering if we use the game atlas

- We were worried that once we move to gif-format for officials, we won't be able to skin e.g. new stances as usual
	- But there won't be official maps to read from anyway
	- So we'll have to prepare an intercosm for skinning like this
	- And then we can just pass the generated png file paths as always
	- In any case we'll probably port it somehow or even use another tool (?)

- Gral gif is colorized yellow when neons are enabled, probably due to wrong light calculations
	- We should probably check on windows first

- Randomization like on assault rattle should be at rendering time instead of animation time

- Btw first thing we should do for atlas optimization is have a separate neon map atlas

- Editor sprite resource will have a vector of frame metadata
	- It will always expand and will never implicitly shrink if there are suddenly less frames in the gif file
	- But we don't necessarily even need it for now, at least until we enable per-frame editing
- Actually how is size currently read?
	- We will literally read the frame count in the same place
	- Size is always overridden as well in the resource
		- It's just that nodes can override it
	- No need to reuse get_size because it's actually implemented to be fast

- Okay, how do we unpack these animations?
	- The problem: viewables access specific frames as assets
		- We could virtualize the paths
		- This would break the old editor a bit
	- However!
		- It doesn't matter for cosmos logical assets
		- But we regenerate viewables in editor too so it should be correct
		- Path virtualization probably the way to go like .gif#1
			- This will only happen in viewables
			- What do we need to fix for this to work?
			- Basically atlas generation
				- Uhh it also breaks neon map generation
					- Alright fastest and easiest and cleanest would be to just save the gif in caches instead of virtualize it
						- But at which stage?
							- Given the path in cache would be deterministic, we could unpack it in atlas generator
							- The files do not have to exist right away, they will be generated by atlas and thus will exist once they're needed
							- And ad hoc will anyways read from the gif file directly
						- Not necessarily the most performant with the encoding overhead but screw it?

				- Atlas is already able to read gifs
				- It just needs to repoint these virtualized paths to a proper atlas sprite
			
- For officials, we can query meta lua files to determine how many frames in gif there are
	 - Unless there are frames without metadata
	 - Okay we could quickly query the gif info, probably the best way to go
		- Nah actually I think every official has meta lua
		- But we might need to query this either way for the custom gifs

- We can query the meta files for officials, but what about custom gifs?
	- Maybe we should handle them first
	- Virtualization will be necessary either way
	- We'll also need to know the number of frames
		- Even just for editing props (which would normally be a very special case but still)
	- So we'll need to know the number of frames at asset creation stage in editor logic
	- So we'll probably need to know the number of frames either way
	- We can read the number of frames as metadata and store in filesystem resource next to thumb id
		- Later we can even use ad hoc atlas to display it to edit some per-frame properties

- We really shouldn't worry about the old editor for now, we can wait this much
	- Only important that we don't break the abi for reading current binary maps into the game if e.g. we need to push a quick unrelated bugfix
- In that case we might need to leave the png files that end with 1 2 etc
	- But we can freely modify the official flavors

- Animations
	- Okay so we just need a means of translating some editor animation resources to proper cosmos resources
		- Contrary to classic editors we won't make animation a vector of sprite resources
		- It will instead keep a single animation resource with properties for a single sprite to be applied to all frames
	- How do we plug them into object-per-sprite system?
		- Can't we just make it a sprite?
	- Technically for editing guns or walking animations we'd need to skin/edit state of every frame 
		- But for environment objects we should just make it abstract
	- Used only in a) animation component or b) particles defs
		- animating decorations/organisms (animation component)
	- why can't we just create animations for viewables as usual?
		- we probably can and will
	- We only wanted to templatize the particle types because we need a separate means of keeping them in the editor state
		- could be introspectively assigned
	- What about pathed assets?
		- Do we want to keep separate forms of animation, i.e. separate pngs and gifs?
		- I think just one will be enough
		- It's good if we force small number of colors anyway
	- How do we create the rest of resources like officials?
- So problems to solve
	- For adhoc there can be a separate system that just keeps track of the animation and feeds proper changed coordinates
		- Without virtualization on the atlas generation level
	- We won't recreate the atlas every time, we'll just pass different coordinates
	- Okay so this is done


- Aquarium creation logic is important because otherwise we won't be able to faithfully reproduce de_cyberaqua and fy_minilab

- Finally test that gif->on-scene animation pipeline
	- And maybe mass-gifize some png sequences like fish
		- Even just for the sake of nicer view for official resources

- Static collider entities
	- static_collider
	- just a physical material
	- will arguably never need some specific interaction logic as this could simply be in a separate non-physical entity
	- it's not meant to be much of a dynamic thing either
	- Resizing/Rotating will just reposition vertices
		- Or will it? Maybe let's just apply these parameters like with normal sprites
			- Although size is non-applicable but we can still have scale
		- If we ensure standard objects *cannot* be selected along static colliders, we can run some custom resizing logic
	- Resizing/zooming might complicate our snapping logic
		- That will only affect how we calculate the offset though, it will always be just a translation
		- But still we'll probably want to preserve that information about how a shape was initially

- A radiobox-like tabs for "Object layers"/"Collision layers"
	- This is the final boss
	- A simple on-scene vertex editor will be VERY handy and it won't really be that complicated
	- It will let us avoid implementing the whole "detect neighboring static walls" and just let the mapper map it out themselves precisely
	- Allows for some cool skinning options
	- This could even be a tab in layers view?
	- Although theoretically nothing stops us from having these static walls as nodes like all others 
		- Still it might come in handy to be able to instanty switch between these two modes
		- It will just make these collider entities visible or not though
		- And at most change some default mouse behavior
	- A full-blown on-scene geometry mode will also be essential to have soldat-like irregular maps
		- Lack of this is what made de labs so squary
	

	
- Layers with all-default values should be deleted once the last element is removed


- Once special sprite effects/continous effects are live people will rarely use playtesting, obviously we sill need to do it

- Asset-based Icons for special resources
	- Do we take it from the game atlas?
		- Technically when we prune the atlas it might be missing
	- For project-based resources, there will always be a resource with the sprite from which we can read 
	- For officials we could refer to the initial scene
		- Problematically this won't necessarily be in the game atlas
			- Especially if it gets pruned
			- I wouldn't prune officials though because technically they can always be requested by some scripts
	- Taking them from the atlas will work and I'd avoid pruning the special resource related assets

- Listing weapons - they need some special creating logic
	- Similarly, magazine listings
	- We'll basically have separate categories for:
		- Firearms
		- Ammunition
		- Melee weapons
		- Explosives
	- In particular those will not appear in gfx
		- If so, how do we go about creating them in custom maps?
			- It's not a worry for now technically
		- Initially, they WILL be mixed with other environmental objects and you cannot do anything about it
		- Nothing wrong with that
			- Yes we can hide those in officials because the resources are already madew
			- But there's no way to hide them in the Project tab
			- Will be your responsibility technically to just categorize it properly and make sure you don't just drag and drop the graphic instead of the entire weapon
				- Will be easy to notice though as there won't be a magazine
		- How else will you define sprite properties like neons etc for them as sprite resources?
			- You will have to set their domain to physical to edit their physical properties though
		- The icon for the special resource will display just the weapon but could even display the entire thing with attachments later
		- The weapon creator will just accept several parameters and create the weapon straight away with the magazine resources too
		- Problem: recreating the weapon from the creator might leave us with abandoned magazines
			- A problem for later since officials wont change
			- Still, that creator will mostly be for asset specification
			- Later only small parameters will change not the whole concept with assets involved


		

- We can make fishes into gifs to test the GIF animation pipeline

- For now we can leave the audiovisual speed on 1 because it doesn't break anything and looks cool
	- We could even simulate a passing global time since I think that's what the rainbows are based on

- Don't list physical materials for now in the filesystem
	- It will only make sense once we can view them or create new ones
	- for now it's enough they'll be choosable from some list
	- Once we do list, a simple tooltip when someone tries to drag&drop e.g. a physical to a scene
		- Physical materials cannot be instantiated as nodes.

- Also the rainbow fish thing will now be handled with separate files
	- We can easily optimize it later by detecting duplicate entries in the atlas (by hashing for example)
	- We could also have a naming convention for copying files but that would be overkill

- Technically aquarium could be templatized without fish parameters
	- We can later just drag& drop from the organism section
	- An organism area will be a special resource too

- Something like aquarium could still be parametrized by entities though
	- It will first need to be created and put on scene
		- Since a resource cannot contain references to an entity

- For differentiating between stuff like dragon rainbow fish and rainbow fish we'll use a flag "disable special effects" per-sprite
	- Okay but it's still problematic because they can't be used as separate flavors/resources
	- So can't be passed as separate arguments to an aquarium for example.
	- As it stands we can't even list them properly as resources
	- At some point in time we might have virtualized resources
	- For now we'll just have to use a single flavor

- Colorize icons/resources by their colorize parameters

- Let's do the same for weapons so that we have a nice listing of them

- We need to properly decide which official resources to list
	- However what we've done will already help us a lot in that we won't have to port particles itp

- In case we want more standalone particles we'll just add them
	- As for singular effects.. technically these are separate resources
		- Thing is something like a muzzle shot effect or any burst is different from environmental which should be normally viewable through official resources
	- We shouldn't worry about it for now, we won't be creating a particle editor for a while still
	- Does the same problem happen elsewhere?
		- Also, materials
		- So we could just have separate categories, sound effects and particle bursts apart from "decorations"
			- decorations pretty much imply they're static
	- Sounds would technically be similar, there are bursts and ambiences
	- However someone creating an environment in the editor won't bother with "bursts" and "sound effects", they will only deal with these environmentals
		- Well unless they're making an interactive object somehow


- Okay so the first challenge will be to list all these official resources somehow in the resources view
	- We can keep the old generation logic
	- We should just create a manual list of resources to list
		- We don't want like muzzle sounds or magazine pics because they won't be used
		- Even though they could technically be used in creating new special resources later
	- We could virtualize the folders
	- Some foldering would only be needed insofar as special graphics are concerned, we can leave it for later

- As for whether we should have a special Domain for weapons/special objects..
	- We'll worry about it when we get to this stage
	- Because we'll only allow creating weapons/particles/phys materials later
	- We could even decide to make it a separate special folder
		- so that normal graphics don't have additional domains to choose from

- By default all official resources will be created. What do we do with the redundant textures that inevitably get included?
	- We need to prune them once on cosmos generation stage
	- I see even de_labs2 includes road sprites which technically sucks
		- but not a problem for now since it's worked like that for a while
	- Yeah at some point we'll need to run a dependency graph and remove viewables that aren't referenced
		- But we'd need to do this anyway even if we didn't create all official resources at abi level


		- Reusing official content creation on ABI-level
			- Pros
				- Editor ready sooner
				- Finer control for now and won't screw up creation process
			- Cons
				- Missing out on easier creation process for official resources
					- Though one could argue that doing it at abi stage is more... immediate? (Won't have to create translation logic for a new feature we want to test)
				- Won't be able to reference it/properly visualize it from the editor as a resource?
					- WILL be able to reference it actually
					- and will be able to easily create an ad-hoc icon logic
					- This is the only means of visualization we need
				- Won't be able to view properties
					- It's not important
				- Also won't be able to create a custom resource basing on it
					- Unimportant too, will encourage creativity
			- Note that even if official gun creation happens in abi...
				- ...we can still have custom gun resource creation logic without being forced to use it to create the official resources


		- Can we do it though?
			- Problematically, guns/particles/materials etc will refer to official resources which will be reference-able from the editor
				- Can't we just visualize and somehow associate them? Given their properties won't be editable and don't even have to be viewable
			- E.g. we wouldn't be able to do it with sprites because we wouldn't even be able to list them
			- We already have a separate 
- We could technically hold enum tags in editor's sprite resource struct
	- If it's set we know it's also official
	- However we'll already know it because official resources are in a separate struct

- Do we have another domain for special objects like aquariums and guns?
	- They mix objects from various domains

- It would be good to implement something like aquarium early to finally clear all that confusion
	- As well as weapons

- As for officials we also need special objects with parametrization
	- Like aquarium with parametrized sizes
	- This doesn't have to be visualized in the editor except on the cosmos stage
	- It can just be tagged somehow
	- Aquarium will be parametrized and won't have to reference any specific image ids
	- Only specific non-special official resources might reference other official resources
	- What about lights? Do we just have a tag for them too?
		- I don't think lights are referenced elsewhere though in officials so they can be viewable/editable

- Major problem: reconciling official content with created content
	- Can we use the existing functions for populating the cosmos with official resources?
		- And then e.g. filling it with the custom content from the map?

	- I think none of the special resources will be editable for now
		- We don't have to write serialization for anything that we provide as official
			- It's enough that referencing it is implemented
			- So we can have physical materials natively for now
			- All physical materials will be official for now so we don't have to worry about serializing the collision matrix
				- We'll determine later if we want to have a global collision matrix or just per-material entries because it won't be editable for now
			- Same with particles, we'll just use the officials everywhere and not worry about serialization
			- So that will just be a copy-paste mostly

	- For serializing stuff like ids we should pass a lambda to write_json that has all the required references to properly map the id to a proper name
		- For now let's have the default modifiers embedded in resources themselves and let's see how far it will take us
			- Should work for entities at least since we have no component with modifier, how much really a single sound effect would be reused among flavors?
				- Well maybe apart from stuff like footsteps
				- okay but still different sprites for each of these and thus separate fields with modifiers possible
		- Let's really not store std string directly there


- Alright let's now try creating various entity types like lights and sounds
- I think we'll first implement animations because we'll need them for the particles

- Yeah I'm thinking let's first do those complex topics 
	- that will let us finally strap everything into place

- Could show a full preview when hovering specifically the icons in filesystem

- shift + select in layers selects all
- that multiple selection might come in handy after all to e.g. set to physical multiple crates at once
	- and to drop multiple objects to scene from resources if they are related like those floors
- properly cast light on wall-like objects, perhaps some filter/render layer?

- For particles we might just templatize the particle effect by the image id and animation id
	- We don't even have to explicitly instantiate particle types as the defs are included in hpp

- Handle throw_through later, we'll make it just see-through for the moment
	- we'll have a separate filter for that probably

- I would say ultimately physical material is for damage sound/particle effects
	- Although maybe let's leave that ricochet there after all

- What we'd really want to avoid is to having to specify collision sounds twice because we want different restitution/density on say another wooden body
	- max_ricochet_angle is likely something we'd like to unify, though



- Materials
	- Can't we for now have a set of defaults without it being editable?
		- This could be officials

- Technically, it's the nodes/entities that should have modifiers, not resources/flavours
	- We need to fix the particle decoration implementation
	- but we need to preserve bincompat for now
- Similarly sound_effect_modifier is more like the default properties like the particles' emissions, a modifier would just be gain or sth




- Screw that mouse confirmation for now, encourage enter usage (lol)

- Another special resource: materials
	- But only physical, we won't create a separate material for grounds as it's just a single sound

- We won't forget the possible actions since all of them will have key shortcuts
	- But we need to describe them in tooltips over the toolbars and buttons anyway
		
- Watch out for pixel imperfections. List of possible fixes:
	- We've deleted aabb calculation from the mirror entities command.

- Once we come to prefabs, it should be easy to rewrite these transform commands to operate on nodes
	- We only really need a way to access sizes and positions (as shown by the data that's backed up for undo)

- It seems to me that it's not the worst pattern - all transforming logic working on the generated scene, as opposed to abstract node entries.
	- This is because node transforming logic works on *apparent state*.
	- E.g. for selector, with special stuff for prefabs you can't be sure how something is going to be generated on-scene
		- so you'd anyway be working on entities there and reading back

- Now for a fun question: can commands really contain entity ids?
	- Since they're not saved anyway...
	- This would spare us the initial entity->node translation
		- which would need to be translated back anyway
	- I guess entity id creation is deterministic
	- A million dollar question: how do we revert it? It would seem dirty to save just the state of entities and then read-back on undo to the nodes
		- Because there's no guarantee we'll go to the original state on revert
		- Wouldn't that work anyway? There's no magic involved

- Remember that porting those moving/transforming things will be the hardest.
	- The hardest work will be done by then
		- next hardest is pretty much just state i/o
		- then duplicating/deleting etc

- Now about the moving and transforming...
	- It would be extremely complex to port it to the node structures
	- Problem is it works on individual entities
		- What about prefabs?
		- Prefabs will possibly have a completely different resizing/transforming logic

- Hang on. If move_entities_command accepts a single delta, can't we use it to simulate moving all of the entities (prefab included, just making sure that it's selected in its entirety)?
	- Resize is harder

- After all we could use the legacy commands to let the user "choose" so to speak

- reference_point is just mouse position in the resize command
- Translating back from resize command should be relatively easy
	- It's clearly only touching sizes in overridden_geo + the independent transforms

- Let's not worry about prefabs for now


- Looks like the sound icon selection is broken in the original editor too
	- Light icons work so let's first test that

- Don't worry about atlases being too large with there being too many official assets
	- It's enough that we generate miniatures in memory for the ad_hoc atlas, this should be trivial
	- And we'll implement the occurence detection anyway so that only needed textures are getting into viewables
		- The only problem with that is that when we drop a new object onto a scene, the atlas will regenerate
		- But that's really a small price to pay


- Can we truly assume that there is one entity per node and vice versa?
	- What about prefabs?
	- Well, this is pretty much only about whether we want to rewrite the entity selector to use nodes
	- But I think this would only complicate stuff
	- We can translate it to inspected node ids with do_left_release
		- just like selected_ids is assigned

- Dragging from layres gui will duplicate
	- This will also play nicely with prefabization
	- A layer will have an option to prefabize
		- It then changes all layer objects into an instance of a prefab
		- And creates a new prefab in special resources
		- But that instance can too be duplicated right away by dragging it
		- "Prefabize visible" which only takes the visible ones

- I'm thinking a weapon should be a special resource, not a sprite

- Okay, how do we squash all the:
	- Non-file resources
	- Official resources
	- Into this single window?
- It would, in the end, probably be the easiest to just have separate files per all resources
	- Although we have to anyway enable creating new folders for this easily from the editor
		- If we do, then there's no point in keeping these folders physically?
			- maybe there is to not have another layer of separation
			- We'll just have separate official and non-official
			- Plus you can group the prefabs and all related data together in the filesystem
	- Like in unreal, we could have a separate container called "Specials"
		- we'd then have four

- Actual json files for lights/prefabs/particles:
	- Pros of 
		- Easy to group together
		- Easy to edit/version externally
			- Well, it's versioned in project json as text anyway
	- Cons
		- Atomic save becomes harder
			- Harder to know whether a project state is valid
			- This won't be so uncommon since people would be able to move stuff in the filesystem
		- Have to hash these again

- I think for now we'll just go with a linear list of special objects
	- Future compatibility considerations?
		- Paths can be easily added as an additional parameter later
	
- In UE literally:
	- de_cyberaqua
		- gfx
		- de_cyberaqua.json
	- Official content
- Well, we can use separators anyway to not create another level indentation
	- Have two ticks actually to freely choose whether to show either 
- Alright but question remains how do we fit specials in there?

- Alright
	- A tab under the filter:
		- [Project][Official]
	- Filter, if active, searches both

- For Lights, Prefabs selectables etc just make a single Plus sign to create new
	- Dropdown menu appears

- Or a separate window for Specials?
	- Pros:
		- Doesn't clutter since we don't use it much
	- Cons:
		- Can't easily search for everything which sucks
- The only real con of an integrated specials view is that it's unintuitive having + sign above resources when we can't really add any new

- Alright let's keep it in one window for implementation simplicity if not anything else

- Introduce iconed entities (nodes) before implementing selections?
	- So that we may keep most of the logic
	- In that case we have to implement "Special" resources beforehand

- Sprite: nazwa noda (bez .png)
	- Show resource
- Albo pokazujemy od razu propty, ale tez show resource sie przyda

- W miarę wcześnie bym zrobił jakas prosta wersje tych multiple selections
	- Czemu? Zeby potem nie musiec przepisywac od nowa wszystkich perform_editable_gui
	- ja mysle ze w komendzie po prostu wektor idow + calosciowych obiektow zamieniaczy
		- a tam gdzie perform_editable_gui po prostu:
			- najpierw sprawdzasz common value i nadajesz kolor
			- potem jesli edited, to robisz fora po wszystkich edytowanych i ten property ustawiasz
			- mozliwe ze tu jakies makro wejdzie bo tego pola nie przyjmiesz jako argument funkcji
				- tym bardziej jesli to jakas tablica bedzie
				- tylko z tablica raczej bym nie robil makr jakichs tylko moze lambde na operacje?


- Ctrl+Z should properly interrupt the drag&drop to scene
	- Redo won't be do any harm as it will be the newest command anyway

- First let's convert the project to cosmos so we can see what's happening
	- Co z oficjalnymi?
		- Okej, ostatecznie nie bedziemy mieli funkcji ktora tworzy oficjalne bezposrednio w intercosmie
			- Tylko jedynie wypelnia ten resource pool bezposrednio w reprezentacji ktora potrzebujesz
	

# Creating the viewed cosmos from the project state

- When drag and dropping on scene, we could either
	- Preview the pasted entity live by refreshing the cosmos constantly
		- This is cleaner and is better UX
		- But less performant?
		- And how do we determine the layer?
		- This could be a continuous create node command that only changes the position and target layer/index
			- just like widgets are continuous
			- It's only first posted however once the mouse is over the scene
				- and would have to be deleted if we go back
				- we'll later worry about recovering the inspector state
			- it wil properly show the target layer position too
				- we'll need to scroll to it though
	- Or just show a ghost that approximates the position


# Stuff

- One-click could select it, double-click toggle collapse?

- Eye would indeed be better on the left, to not mix it with the object icons

- There are many possible actions upon the layer:
	- Rename
	- Collapse
	- Navigate to the center
	- Select all

- One-clicking a node certainly opens an inspector on it
- Double clicking could either navigate to it on scene or initiate rename

- Renaming should be quickly accessible to encourage it

- Multiple selection
	- Generally we'll need it for easy moves of nodes between layers e.g.
	


- Note we MUST use "undo_last_allocate" instead of "free" for nodes
	- Because freeing and then allocating on redo might possibly invalidate references earlier in history
	- Similarly for create/delete prefabs!
- Similarly, if we are to clean unused resources we must also invalidate history
	- Or we can make the clean a proper command! Then not necessarily.
	- But wait, what if we clean, then alt-tab with new resources, and then hit undo?
	- Then the undo_free will fail
	- So invalidate history that is
	- Makes sense because that's the only way to ever delete resource data from a project


- If no object hovered while placing an object
	- If we remember the last layer to which we inserted or interacted with at all
		- Insert to that layer
	- Otherwise create new layer and set it as the last one we interacted with

- We want to be able to select layers to duplicate them potentially

- next
	- layers gui
		- tiled ma dokladnie to co wymyslilismy wiec git a zrobimy nawet lepsze
	- simple resource I/O
	- only then some simple cosmos conversions
	

# Scavenging from the legacy editor

- rewrite_last_change
	- In order to keep command api simple, we might want to have a concept of a "preview command"
		- instead of adding separate functions for "rewrite last change"
		- so until the command is actually finalized, just undo it and redo with new parameters
	- Do we need a separate preview command?
		- We can just keep a pointer to the last modified and not even dereference it, just check
			- Index is a better guarantee due to reallocation

- has_parent in history command - I think not, let's just have singular commands and properly have separate classes for multi-commands
	- it would make for a very confusing ui
- Alright but we'll still need to reuse commands
	- e.g. in a delete layers command, we'd want to use delete nodes too
- No probs, we can just keep a command inside a command
	- and then call redo/undo on the children manually

- As for history, for now we could just save snapshots instead of addresses?
	- although that shouldn't really be too complex
		- well, it becomes complex with various containers
	- We can optimize it later
	- The only problem is that we have other meta that we don't want to be editable

# Everything else

- Screw resource_hashes.bin for now and let's always rehash on launch
	- We won't be writing the timestamps to json because we decided we'd put it into the cached resource_hashes.bin just for this purpose
		- But it only really becomes necessary at scale
			- Can't be too long anyway because it will happen whenever we download a new map
		- We can easily add it later
- Since stamps will be empty (default) they will get recalculated after first call to on window activate - which is called right after loading the json file
	- To be sure we can just not read the content hashes from json (since we'll be doing i/o on resources manually)

- Let's go the easiest route right now and just write all resource hashes and timestamps to json

- Summary
	- Separate pools, official outside editor_project, inside editor_setup
	- Id has a bool whether it's official, a general dereferencer
	- map of path_to_resource is calculated only on the rare occasion that it's needed
	- Do we hold content_hash and path inside structs themselves?
		- It's I guess more convenient
		- because we want to serialize them this way certainly
		- If that's not a problem we can hold a path inside the object, not as a key
		- I think paths as keys look cleaner, there's also one line less
			- it clearly shows it's meant to be unique
			- especially since internal resources will too have names as keys
	- EASIEST: json_ignore_fields. Runtime. We won't get a compile error because a string is serializable
		- screw performance hit, it won't do anything
	- probably augs::json_ignore with an explicit operator
	- can't we just inherit the relevant fields into the full resource? And then cast, I don't know
	- setting path to default so that it's ommitted sucks because it involves copying
		- would be better to just make a runtime if
			- why not then? this is by far the cleanest solution

- Alright so we need to somewhow ignore the "path" in resources
	- Nothing else really
	- e.g. for pseudoids we can just manually read_json from that object into a separate variable
		- and construct an unordered map on the side for just the duration of the json reading function
	- well then, just a struct augs::json_ignore or path_holder?
		- makes code uglier so i'd avoid it
	- we thought about constexpr == const chars but it probably wont work

- Alright so we also don't need to hold the pseudo-id "id" but we need to write it
	- We can write it easily separately
	- the problem is if we need to hold something and not write it, so the path

- Separate pools also makes rescanning for filesystem changes easier because we're not looking up irrelevant data
	- Yeah, so we certainly want separate

- std::unordered_map<augs::path_type, editor_resource_id> path_to_resource;
	- This is really only relevant upon:
		- i/o
		- remapping when rebuilding filesystem
		- so do we really need to keep it? We have to also keep it up to date when allocating etc...
		- I don't think so


- Notice that the only moment that it matters whether a resource is official or not is json serialization
	- Because it determines whether we serialize it to external_resoruces or not
	- So it's not a problem if during serialization we do some recalculations to determine if something's official or not
	- We need to allocate the needed official resources somewhere anyway
	
	- Why not just go the easiest route and have a separate pool of official resources outside of the editor_project struct?
		- Always allocate all official resources
			- note this still should not impact our performance
				- only images used on the scene will be loaded to memory
					- later we'll optimize so only thumbs are loaded for the filesystem view or something
				- the created resource structs will be useful for indexing in the filesystem
		- the only nuisance is that we'll need a general dereference that takes into account both pools

- Instead of the whole fukcery with pools why not just identify every resource with a string?
	- pro: just std::unordered_maps
		- although once we have apis this doesn't change a lot
	- pro: solves the problem with identifying official ones
		- since we have the @
	- pro: 
	- con: have to use pseudo ids right away
	- con: have to recalculate every time we dereference instead of centralizing this
	- con: performance vs integer-based ids?
		- but when does it matter?
		- The only complexity that matters is json loading complexity for now
		- And for that we'll have to dereference those ids by strings anyway
		- Performance shouldn't matter all that lot
		- And it's about runtime data layout so this can always be improved later


- Alright, what about differentiating custom ids and official ids in binary?
	- For a unified api, we could allocate the used official resources in the pool
		- But hey, we have our own id structs, so we can easily pass a boolean there
		- This saves us a lot of pool manipulation operations
		- This simplifies data layouts too because we don't have to hold that "is official" data in resources themselves
	- So that an id refers to either official or not
	- But we have to determine which ones are used
		- Upon writeout, how do we detect that a resource is official and thus does not need to be saved?

	- In the cosmos, it does not matter whether a resource is official or not

- Co do resourcow 
	- i tak musisz hash walnac wiec nie ma po co sprawdzac czy jest default przy pisaniu
	- rescan
		- usuwamy te co nie sa 

- Dragging stuff into the scene
	- Docking breaks it because we basically have an imgui window sitting there
		- and it fetches all inputs probably
		- we have to make it somehow "no inputs"
		- or make a corner case for whether imgui input should be fetched
		- in any case we should be able to 
	- I wouldn't worry for now about background becoming invisible with particular dock configurations, it's a corner case
		- solved: ImGuiDockNodeFlags_NoDockingInCentralNode
	- Btw it's better with an empty central node because
		- Handles inputs properly out of the box
		- Won't unnecessarily occupy space with the single tab 
- However if we properly made the scene another window, we would right away solve problems as:
	- showing the "unsaved" mark next to the tab name
		- jakby sie uparl mozna nawet przy Project dac albo nazywac Project nazwa projektu
	- right away implementing mouse navigation with offset
	- I still think that could be done later

- let's implement the last opened project path for speed of iteration
- Haszy chyba na razie nie potrzebujemy, tylko do podpisywania

- Later also do some unit tests for json readwrite

- Let's leave it at "arena" nomenclature
	- Will even easier to find in code

- Open selected/Create from selected
	- Name your clone/Name your new arena

- "Choose a unique name for your map, be creative!
If someone has a map with the same name,
they would have to delete it in order to play yours."

- Reading the about section from the maps
	- for now let's just read them whole

- Creating new project

- co jak po drodze jest symlink? my tam robimy przeciez tylko weakly canonical
	- chyba ze weakly rozwiazuje symlinki

- editor_setup -> editor_setup
Nie nazywajmy tego arena editor tylko użyjmy najbardziej oczywistej nazwy

- Breaking changes that e.g. change signature verification scheme could be done by changing the builds/latest to e.g. builds/latest-2
	- In practice, builds/latest would always have the latest version so that it's easy to link it for people who don't have a binary yet
	- but the binaries would have defaults that point to symlinks like builds/latest-1, builds/latest-2 etc.
	- Not for now but good to have this planned

- masterserver -> serverlist maybe
- test code_escaped_nick against \`

- Balance datum, either way more expensive or less velocity
	- it's way more powerful than e.g. szturm

- A centered popup with the after-update message
	- The current right-bottom notice is hard to notice

- Fix that camera flying somewhere upon the last kill

- don't give money to spectators
	- just set the money to starting whenever a team is chosen

- item overlaps: choose the item whose overlap point's distance is the smallest to the query center
	- we'll be able to move the center around a circle with just the mouse so it should be cool

- manual deployment only after all of the binaries are built
	- so that the server and clients can go up at the same time

- maybe: properly trim the crosshair raycast to the end of screen and change its color accordingly 
- sort out the crosshair rendering when zoomed out (though we're going to redo the whole thing soon)
- has_size_limit easily just check if the max_size is in sensible range
- sort out those damn max_size/size constexpr traits or get rid of them altogether
	- there is one in explicit pool(const size_type slot_count) {
- move is constant size vector/string to traits
- sort out the max_fps problem

- Check if we don't assume too much about the subject in damage indication system
	- or in draw sentiences hud for that matter
- both should abort early if the subject is not sentient
- be wary of any state that you hold in damage indication system
	- because the ids might change identities or become dead suddenly

- Let only the headshot sound wait for confirmation
- Fix first blood being counted during warmup
- Make the shield a bit lighter for balance

- Item emphasis implementation
	- First pass: shadows
		- Just render all items with pure color shader, with varying intensity (the closer the item to ground, the less shadow)
	- Second pass: actual items
		- Just render the item
	- Third pass: white overlay
		- Just render all items with pure color shader, with varying intensity (the closer the item to ground, the more highlight)

- Emphasize the items laying on the ground
	- Like in hotline miami, they might fly a bit to upper-left direction
	- with a little shadow behind
		- just use existing color overlay shader for that - it will be rendered as the first
	- screw velocity check, just identify the physical filter for it to certainly be the lying item (we don't want to apply it to grenades)
	- a little cyclic shine on the top (might also be done with the overlay shader
	- We could disable emphasis for the empty mags

- We have improve the nomenclature for playing event sounds in bomb defusal code
	- Maybe when we refactor the mode code

- Show a flame icon when we apply damage to a burning corpse
	- Big one on ignition, smaller ones when we help

- fix this..
	- const auto next_explosion_in_ms = rng.randval(0.f, cascade_def.explosion_interval_ms.value);

- Multiple "camera schemes" in settings
	- Fluid (current, hotline miami-like)
	- Rigid (crosscode-like but we'll have to somehow reintroduce the nice feeling of weapon recoil kicking - somewhat artificially)
		- miniscule dead-area like movement will stil be good

- More cool dash effects
	- A transient border-silhouette at the place of initiation, with a sense of direction
		- A pure color highlight?

- Dash animation
	- With all body parts skinned of course
	-Show Only shows when not shooting

- Hide enemy health bars when they are under ferns or other object that's supposed to hide them
- Maybe a little more shake for the explosions of electric triad?

- Killstreaks/combos could add HP
	- To balance one versus multiple situations
	- But people could fight over the frag then

- Blood traces after tearing the body would be cool
	- These need to be entities for proper sync
		- Cool if they'd occlude the neons

- Make dead bodies respond to hits and have some 80 HP as well
	- Then they will explode to bloody pieces
	- If someone has 1 HP they will get torn to pieces on awp shot or even scout/deagle shot
	- tearing apart causes additional loss of money?
		- it should cause high screen shake like a grenade
	- Current damage sounds could be used for damaging the dead bodies
		- and we could use a more snappy sound for normal damage
			- e.g. glass breakage
				- maybe something combined with current analog impact sounds (albeit in way shorter version)

- Hold symlinks in user/ so that they don't get deleted on update
- Position the nade progress circle correctly when reloading

- First reload the one that has less ammo
- Consider not stopping the reloading context on shooting e.g. the gradobicie

- Use ESC to close the rcon please

- We should just show a complete inventory screen on pressing Tilde
	- Escape then goes back to the game

- New inventory system
	- Picking up a weapon should also pick up mags in close vicinity
		- Hover too
	- We can't cycle through same weapon types by repeatedly pressing same button
		- it's because we want to allow akimbo
		- Would make sense with nades though
		- But we have enough numbers to just assign nade types separately

- The fact that we don't want to allow to holster a heavy weapon without a backpack is making it incredibly complicated
	- We might let it switch for both of the weapons
	- or for primary

- Without backpack:
	- Can always have a maximum of two weapons
	- Scenario: heavy gun picked
		- Q just drops it

- Vote pause the game
- Simple commands for chat
	- time

- Release notes
	- release_notes.txt holding last 100 commits
	- Updated on each upload

- Why are indices wrong in shop?
	- Though perhaps it's for the better?

- Show cursor when chat is open
	- And allow for scrolling/selecting the history

- "X killed Y with Z" chat notifications
- Really fix those spectators being kicked
- Dump those logs periodically to files or implement this dump command
	- For masterserver too somehow

- A progress bar for the rocket launcher

- Leave the project selector final touches for later
	- Sorting
	- Filtering
	- Create new arena window
		- We'll only really know what to put in there once we have the editor done

- Makes sense to keep arenas outside of content/official 
	- since we might want to have arena-specific files that won't be intended to be used for custom maps
		- like warmup themes. Not that it will be disallowed but it will encourage own creations

- Well.. it's a pain but we'll still somehow need to order entities within an order
	- Unless we spam with these flavours like crazy
	- Well what if we want to have the same decor in both background and foreground? Makes little sense to make another prefab for this, better just auto-generate two different flavours
	- we can always put the sorting order in a component instead of invariant
		- since, at this rate, it's pretty much something like a transform
	- We could screw the sorting order and just sort by ids
		- Fast, though is this what we truly want?
		- Not really. Consider different entity types

- New entity type: tile layer?
	- Shouldn't really be rocket-sciency to support animations later
	- Do we mix tile sizes? Preferably not...

- We'll move insects and other stuff to official only once we have a general outlook at the editor
	- No point in doing this now

- Area marker variation (?)

- All obstacles should be on a well-defined layer
	- like in jj2
	- Well what about detached heads?
- The point is what the user can see
	- Ground and Foreground will not be physical
- What about mixing physical and non-physical tiles on the same tile layer?

- I think we need to let go of the concept that "render_layer" will directly correspond to the stuff in "Separators"

- Do we really want to expose "wall lighting" option to ground layer even when it will make little sense?
	- Perhaps make it only appear for the wall layers and all wall flavours will have it on by default

- Notice that we will have to make likewise "separators" for callout markers, lights and whatnot
	- So shouldn't they be render layers too?
	- We could call it a special layer
	- All special layers will be a heterogenous tuple of arrays of possible id type vectors
	- I'd divide it into tabs
		- Foreground
		- Background
		- Special
	- Though it would be enough to just have a single sub-layer of lights and of markers
		- instead of whole separators

- Maybe instead of "glass obstacles" and "solid obstacles" have just solid obstacles and a bool whether we want to apply additional wall light
	- similarly with foreground?
	- but the glows would anyways be rendered on top so it'd be counterintuitive
		- similarly with neon erasers
		- we can have though instead of SOLID_OBSTACLES_OCCLUDING_NEONS just a single solid obstacle layer and just a flag if to occlude floor neons
			- since it will anyways be shown always above the ground layer 

- Sentience -> Character please

- Fix version numbering on MacOS
	- It's probably because of the shallow clone

- Left-handed option

- Fix those mag refills please

- Fixing wallbangs and wall teleportation
	- A short raycast sensor in front of the player
		- Detect all convex shapes of walls in front of him
		- Just take all vertices of all detected convexes
		- And create a single trapezoid body
	- Don't do raycast
		- simply query the  right before the physics step (after applying the crosshair's direction to player rotation)

- Leaderboards shown in the main menu
- "Host a server" button right in the arena editor

- Look for some filesystem implementation for imgui
	- We'll also replace the system viewers? At least some of them?
		- Only reveal in explorer will be useful at this point

- A setup needs to be able to launch another setup easily
	- A callback with a launcher passed?

- Rock the vote for modes, schemes and difficulties

- We can pay someone to properly re-map all of the official maps in the new editor.
	- The ultimate test.

- Wtf with those mags appearing at the center of the map?
- Also you can still sometimes get a magazine in hand if you buy HPSR with little inventory space
- And you still sometimes need to reload weapons after the round is restarted

- Player can see the opposing team's inventory post-mortem

- The aborted sounds end abruplty if we have less than 1.0 master/sfx gain in settings

- (Update script) Sync config.lua against the user folder in the home dir, not in hypersomnia dir
	- otherwise we have to call vim_build AND vim_run later

- Post-map-format-fixes
	- match begins in seconds for de_labs2 in bomb_defusal.cpp
	- ultimate wrath of the aeons FX fields are used for electric triad

- Energy consumption fix: sleep the remaining frame time

- Look for imgui demo's Console for a nice copyable log text field
	- separator under button

- Fix flashbang sound volume

- Camera should also ease towards new positions instead of resetting completely

- Fix crash when the read value is a value and lua readwriter expects a table
	- and the other way too
	- just handle an exception probably

- Turn off logs of masterserver and browser later

- Implement sending the current version to the server

- Remember to not send goodbye for when the servers are automatically updating

- Apparently, clipboard prevents connection to any server...
	- Because it can't bind the socket. lol

- The masterserver should dump entire state before restarting for update
	- To give servers a chance, set time of last heartbeat to current time after starting up

- It's not really a problem if the server list is recalculated in the background.
	- The bandwidth used is minimal, especially since we've imposed a limit on the number of packets there.
	- So let the advancer just go all the time.

- later determine why heartbeats mismatch every time

- We should give up on opening hosts that don't respond

- Just pass a lambda for ingame menu which buttons are available
	- Later we'll properly hide them but for now they'll just be inactive

- IPv6 fixes
	- ip-agnostic find_underlying_socket

- advance only advances pings and nats already requested
	- but it is only the imgui-performing function that requests the pings and nats in the first place
		- and only if it detects that there is a need for them and a sufficient interval has passed

- For now, don't save in custom config after connecting from the list

- If someone connects at the last slot available, and there is no master RCON on the server, kick them if the rcon doesn't match
	- So that we always have a slot registered for RCON

- Detecting servers as official ones given just masterserver list 
	- The client anyway has to resolve official server ip addresses for pinging
	- match ips

- Find best official server?

- Resolve masterserver hostname once a minute or two, asynchronously?
	- mutex onto result netcode_address_t

- just take sockname from the existing connection with masterserver over http
	- when you downloaded the server list

- don't change map structure until we finally make demo player improvements
	- which we need for demo replay
	- or make a branch for demo replay improvements and rebase unto the original commit later

- I think let's just first do nat punch over external ips
	- let's search for how people do those internal ips on the internet

- check how getifaddrs works on the linux server to see if it properly returns external ip
	- Well, it doesnt...

- Later only respond to ping if the request packet is 1000 bytes or so
	- to avoid ddos

- Don't add the option to make MS "assist" your client in connecting to direct ip
	- It will be anyway easier to just search for "Kumpel's Server" in the global server list than to paste an ip
	- Servers most recently hosted will be checked for latency first

- Add RCON to server and client in-game menu

- Server name to server vars
	- server sends it to masterserver next time

- Info about official servers
	- We still want official servers to send periodic info to masterserver 
	- because they will be in the list in the main menu too
	- Website just downloads list of masterservers
	
- Security concerns when rendering on site
	- We need to consider that arbitrary string might be found inside server's name, map name or player username

- Sending server stats

- Website server status
	- Game servers have a built-in http server?

- Leaderboards
	- Columns: Avatar, Nickname, Kills, Assists, Deaths, Hours played

- Setup a simple dev journal at hypersomnia.xyz

- Advanced RCON functionality
	- Kicking and banning users
		- Rcon can download the log
		- Rcon should have a reserved slot to enter in case of emergency
		- Restart
		- A way to view dedicated server stats?

- Advanced demo replay functionality 
	- Optionally be able to add a synchronized audio track for scaling with the time
		- If we recorded the voice separately for example
		- It will synchronize with slow motion etc
	- We can make non-serialized snapshots easily
	- Autodirector
		- Minimize the time we have to spend on editing the footage
		- Highlights of each 10 second durations preceding ends of rounds
			- for compact duel of honor recordings
		- Smooth slow-motion seconds setting in gui
			- augs::maybe
		- Somehow detect action?
			- Or just show the player that is going to be killed next?
	- next/prev round and death
		- textbox with an offset for deaths
		- 5 second threshold for rounds
		- demo replay advance takes a lambda 'has_occured' that takes an enum demo_occurence_type KNOCKOUT/ROUND
		- alternatively steppers may return some metadata with a bitset of occurences that happened, if it depends on post-solve
			- but we can look into knockouts

- Demo replay fixes
	- Money is wrongly displayed on demos
		- I guess it is shown for the the local character
		- we should also show all moneys on the scoreboard
	- Reset timer after seeking during play, because lag will force to advance again

- Let shells not invoke any sound when hit by a grenade
	- to lessen hrtf impact and occurences of sound interruptions

- Update process fixes
	- On update on linux, symbolic links to filesystem handlers are lost

- Advanced update automatization
	- CLI for editor re-exporting
	- Server updates
		- Check once every X minutes or everyday at a predefined hour
		- Remember to only pull an upgrade if both Windows and Linux versions match
		- For now have a rcon command for "Schedule server update"
	- While in the main menu, check for updates once every several seconds
		- e.g. if the server has to restart due to an upgrade, the clients will follow with the update right away
		- also trigger update check every time we enter the main menu from the client
		- New version available!
			- Do you want to restart and upgrade the game now?
				- Upgrade
				- Cancel
					- Automatic update was cancelled.
	- Community map conversion

- Faction win sounds sometimes get spatialized, when switching spectated person

- Spectator fixes
	- Don't force spectated player switch upon match summary
	- Fix spectator sizing after death
	- Show death summary in spectator

- Run thread sanitizer at least once

- Fix openssl errors in build process on a clean arch linux
	- Perhaps just use shared libraries if shared can't be found

- Chat messages bugs?
	- Research chat messages sometimes not working
	- Fix too many chat messages

- Don't kick afk spectators
	- send some heartbeat on mouse movement
	- or just not kick at all
	- actually instead of kicking, just move players to spectator for an indefinite period of time

- Inventory GUI still acts up

- Test what happens without internet connection when launching Hypersomnia on Windows

- Font-scale invariant update window

- Website

- custom chosen ruleset could be a part of mode solvable
	- or actually client/server solvable (arena_handle)

- Ruleset chooser might be a combo too
	- it might only read the names or ids from the ruleset file
	- rcon can set values on the go (later)
	- or load a lua (later)

- Update imgui later for cool features like builtin tabs

- Community map transmission
	- Maybe asynchronously compress the entire map to .7z

- Disallow exporting to lua when playtesting

- Gameplay fixes

- Gameplay bugs
	- The problems with walls
		- Can walk through
		- Can shoot through

- LPM acts as Q when hands are empty
	- Punches when no weapons
- RPM could always punch

- DO NOT disclose a single rcon password to multiple moderators/admins
	- That is because a single malicious RCON holder could avoid responsibility for their misdeeds
	- Instead, just assign permissions to accounts, or just the private IDs

- Do we want to somehow let the user be able to rollback to older versions?
	- How do we even do it? Build retention on appveyor?
	- Not now, certainly not until editor is publicly usable


- We could set a limit to the number of allowed simultaneous muzzle sounds from the same gun
	- similarly for health decrease sounds

- Remember to re-import the new content to cyberaqua after we're done

- We might want to somehow decrease heap contention between threads
	- Best would be per-thread heaps
		- Even better just no allocations
		- Though even something as trivial as draw debug details will do a lot of allocations
	- Use hoard allocator?

- blurred text if zoomed out

- fix area indicator zoom in editor

- In-game tip system
	- Notifications like "can't holster" will be drawn a little above the context tip
		- So that both can appear at once

- Keep timestamps in log structures and, when writing the logs to a file, preffix the log entries with time
	- Will later be good to separate logs via date, for the dedicated server

- Dump logs once every 1000 or so
- Write editor write date to version.txt file
	
- make layer with insects hoverable in editor

- bomb falls outside the map

- Note that message buffer might overflow during resynchro, causing a disconnection

- increase prices of uwota and triad? theyre soo op

- check if export/import of rulesets works correctly

- when post-solving referential, one could see if a similar-enough effect has happened in the predicted post solve.
	- if not, we want to post it, because a predicted cosmos might have not predicted this effect occurring.
	- this could be done for id-insensitive events like effects tied to weapons and characters
	- and not necessarily for bullets 

- when re-exporting
	- fix spells
	- remnants
	- weapons
	- character stats

- Bug: path specification for assets doesnt fully work on Windows

- in editor, allow modifications after re-export
- in exported flavours, identify by filenames and flavour names not by ids

- during akimbo, only drop when the G is released, not right away when it is pressed
	- when G is still held, you can press either LPM or RPM to decide which weapon to drop
	- if G was released without holding lpm or rpm, drop the most recently wielded item as always
	- if G is still held while we have only one item left in hands, still allow to drop by pressing either LPM or RPM  

- in case the gui still acts up
	- always keep this personal deposit open
	- recalculate hotbar on every round start?

- particles don't get properly predicted on deaths sometimes?

- if the referential post solve determines that the predicted post solve has missed something predictable (e.g. a bullet impact)
	- we should somehow try to replay it

- Fix prediction of collision sounds
	- Never predict collisions of remote players
		- This might be important for not exposing tactical information
	- Predict collisions with items only if they weren't just recently dropped by a remote player

- Sync player's change to nickname

- Equipment generators
	- Should simply be markers that are used by the modes, depending on the flag
		- later we'll make the testbed conforming
	- is it important now?
		- i guess clientside prediction is more important
		- though we can plan for state

- should rebuy previous also buy magazines bought?
	- perhaps

- Do something so that we don't accidentally discard work in playtesting mode

- Easily spawn loaded weapons or magazines
	- For now, let instantiation load them all by default

- Simplify workflow for creating new weapons?
	- E.g. remove the need to specify finishing traces

- matchmaking
	- stats persistence
