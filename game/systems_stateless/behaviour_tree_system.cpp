#include "behaviour_tree_system.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"
#include "augs/ensure.h"

#include "game/components/behaviour_tree_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

using namespace augs;

void behaviour_tree_system::evaluate_trees(const logic_step step) {
	auto& cosmos = step.cosm;

	for (auto target : cosmos.get(processing_subjects::WITH_BEHAVIOUR_TREE)) {
		auto& behaviour_tree = target.get<components::behaviour_tree>();
		
		for (auto& concurrent_tree : behaviour_tree.concurrent_trees) {
			auto& tree = *concurrent_tree.tree_id;
			tree.evaluate_instance_of_tree(step, target, concurrent_tree.state);
		}
	}
}