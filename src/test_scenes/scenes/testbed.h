#pragma once
#include "game/transcendental/cosmic_entropy.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/cosmos.h"

class cosmos;
struct all_logical_assets;

namespace test_scenes {
	class testbed {
		void populate(const logic_step);
	public:
		void populate_world_with_entities(
			cosmos& cosm,
			const all_logical_assets& metas
		) {
			cosm.advance(
				{ cosmic_entropy(), metas },
				[&](const logic_step step) { populate(step); }, 
				[](auto) {},
				[](auto) {}
			);
		}
	};
}