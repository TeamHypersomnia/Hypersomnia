#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "ensure.h"

void behaviour_tree_system::evaluate_trees() {
	for (auto t : targets) {
		auto& behaviour_tree = t->get<components::behaviour_tree>();
		
		auto& tree = *behaviour_tree.tree_id;
		
	}
}