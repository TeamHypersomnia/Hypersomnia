namespace augs {
	template <class...>
	class storage_for_systems;
}

class physics_system;
class dynamic_tree_system;
class processing_lists_system;

typedef augs::storage_for_systems <
	physics_system,
	dynamic_tree_system,
	processing_lists_system
> storage_for_all_temporary_systems;

typedef augs::storage_for_systems <
> storage_for_all_stateful_systems;
