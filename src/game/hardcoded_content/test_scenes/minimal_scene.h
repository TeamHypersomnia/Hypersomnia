#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "augs/misc/constant_size_vector.h"
#include "game/container_sizes.h"
#include "augs/misc/machine_entropy.h"
#include "game/transcendental/cosmic_entropy.h"

#include "game/transcendental/step_declaration.h"

class cosmos;

namespace test_scenes {
	class minimal_scene {
		void populate(const logic_step);
	public:

		template <class T>
		void populate_world_with_entities(
			cosmos& cosm,
			const all_logical_metas_of_assets& metas,
			const T post_solve
		) {
			cosm.advance_deterministic_schemata(
				{ cosmic_entropy(), metas },
				[&](const logic_step step) { populate(step); }, 
				post_solve
			);
		}
	};
}