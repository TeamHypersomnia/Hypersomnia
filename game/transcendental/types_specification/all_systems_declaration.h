#pragma warning(disable : 4503)
#pragma once

namespace augs {
	template <class...>
	class storage_for_systems;
}

class physics_system;
class dynamic_tree_system;
class processing_lists_system;

class interpolation_system;
class past_infection_system;
class light_system;
class particles_simulation_system;
class wandering_pixels_system;

typedef augs::storage_for_systems <
	physics_system,
	dynamic_tree_system,
	processing_lists_system
> storage_for_all_systems_temporary;

typedef augs::storage_for_systems <
	interpolation_system,
	past_infection_system,
	light_system,
	particles_simulation_system,
	wandering_pixels_system
> storage_for_all_systems_audiovisual;
