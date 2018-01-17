
- Solutions for "children entities": entities that are cloned with some other entity and deleted with it
	- Stateless calculation of a parent entity and storing results in a parenthood cache
		- Makes some other potentially unrelated state now associated
		- From what do we calculate this?
	- **Chosen solution:** delete_with component that is a synchronized component
		- Most flexibility and separation of concerns; childhood is not really something that will change rapidly, as opposed to damping or density
		- Groups could then easily specify initial values for entities in the group
			- will just be a common practice to set the delete_with to the root member of the group
				- Currently would anyway be only used for crosshair
	- crosshair has id to its parent that is used on deletion
		- we'd still need to cache this value and make more state associated

### Microplanned implementation order (done):  
- rename fundamental to always_present
	- so that it is intuitive that there are no callbacks for adding and removing

Reiteration #1:

- Getting information about success from reinference
Some configurations of solvable significant do not allow for an inferred counterpart that is consistent.  
E.g. if vector of items inside a container is inferred, and one would like to perform a transfer by 
Here's where helper functions might be necessary. A perform_transfer function should only do as much as predict the outcome of reinferring the item component with newly set current slot,
and if it is successful, do set it and reinfer the item.

we consider whole type overrides too complex architeciturally:
- the assumptions about inferred state for entity types are gone; previously it was just generated once and set in stone, always correct
- noise while getting those definitions
- we should discourage solver from ever modifying that state
	- otherwise to look good we would need to always completely destroy that entity and recreate it with components with default associated values

- fix trace interpolation problems (possibly others) in release
	- observation: debug works
		- divergence is consistent after clearing the build folder
	- numerical problem perhaps?
		- "looks like" the interpolation cache may be getting reset too often
	- fixed: a temporary for the version was invalidated
