#pragma once
#include "game/cosmos/cosmic_entropy.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/solvers/standard_solver.h"

class loaded_image_caches_map;

namespace test_scenes {
	class testbed {
		entity_id populate(const loaded_image_caches_map&, const logic_step) const;
	public:
		entity_id populate_with_entities(const loaded_image_caches_map& caches, const logic_step_input input) {
			entity_id controlled;

			standard_solver()(
				input,
				[&](const logic_step step) { controlled = populate(caches, step); }, 
				[](auto) {},
				[](auto) {}
			);

			return controlled;
		}
	};
}