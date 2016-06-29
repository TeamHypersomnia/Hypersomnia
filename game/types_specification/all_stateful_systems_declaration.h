namespace augs {
	template <class...>
	class storage_for_stateful_systems;
}

class physics_system;
class gui_system;
class dynamic_tree_system;
class processing_lists_system;

typedef augs::storage_for_stateful_systems <
	physics_system,
	gui_system,
	dynamic_tree_system,
	processing_lists_system
> storage_for_all_stateful_systems;