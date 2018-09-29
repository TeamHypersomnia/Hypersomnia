#include "behaviour_tree_system.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/components/behaviour_tree_component.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/for_each_entity.h"

using namespace augs;

void behaviour_tree_system::evaluate_trees(const logic_step) {
#if TODO
	auto& cosm = step.get_cosmos();

	cosm.for_each_having<components::behaviour_tree>( 
		[&](const auto it) {
			auto& behaviour_tree = it.template get<components::behaviour_tree>();
			
			for (auto& concurrent_tree : behaviour_tree.concurrent_trees) {
				// TODO: fix behaviour tree storage
				auto& tree = cosm[concurrent_tree.tree_id];
				tree.evaluate_instance_of_tree(step, it, concurrent_tree.state);
			}
		}
	);
#endif
}