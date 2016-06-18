#include "entity_system/storage_for_stateful_systems.h"

class physics_system;
class gui_system;
class dynamic_tree_system;

typedef augs::storage_for_stateful_systems <
	physics_system,
	gui_system,
	dynamic_tree_system
> storage_for_all_stateful_systems;