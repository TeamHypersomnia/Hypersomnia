#include "behaviour_tree_system.h"

#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "ensure.h"

void behaviour_tree_system::evaluate_trees() {
	for (auto t : targets) {
		auto& behaviour_tree = t->get<components::behaviour_tree>();
		
		for (auto& t : behaviour_tree.concurrent_trees) {
			auto& tree = *t.tree_id;
			tree.evaluate_instance_of_tree(t.state);
		}
	}
}