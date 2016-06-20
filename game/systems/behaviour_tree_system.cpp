#include "behaviour_tree_system.h"

#include "game/cosmos.h"
#include "game/entity_id.h"
#include "ensure.h"

#include "game/components/behaviour_tree_component.h"

#include "game/entity_handle.h"
#include "game/step_state.h"

using namespace augs;

void behaviour_tree_system::evaluate_trees(cosmos& cosmos) {
	auto targets_copy = cosmos.get(processing_subjects::WITH_BEHAVIOUR_TREE);
	for (auto t : targets_copy) {
		auto& behaviour_tree = t.get<components::behaviour_tree>();
		
		for (auto& t : behaviour_tree.concurrent_trees) {
			auto& tree = *t.tree_id;
			tree.evaluate_instance_of_tree(t.state);
		}
	}
}