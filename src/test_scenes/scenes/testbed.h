#pragma once
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/standard_solver.h"

class cosmos;
struct all_logical_assets;

namespace test_scenes {
	class testbed {
		entity_id populate(const logic_step) const;
	public:
		void populate(cosmos_common_significant&) const;
		entity_id populate_with_entities(const logic_step_input input) {
			entity_id controlled;

			standard_solver(
				input,
				[&](const logic_step step) { controlled = populate(step); }, 
				[](auto) {},
				[](auto) {}
			);

			return controlled;
		}
	};
}